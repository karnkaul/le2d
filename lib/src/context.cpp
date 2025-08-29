#include <capo/engine.hpp>
#include <detail/audio_mixer.hpp>
#include <detail/pipeline_pool.hpp>
#include <detail/render_pass.hpp>
#include <detail/render_window.hpp>
#include <detail/resource/resource_factory.hpp>
#include <detail/resource/resource_pool.hpp>
#include <klib/assert.hpp>
#include <klib/version_str.hpp>
#include <le2d/asset/asset_type_loaders.hpp>
#include <le2d/context.hpp>
#include <le2d/error.hpp>
#include <log.hpp>
#include <spirv.hpp>

namespace le {
namespace {
[[nodiscard]] constexpr auto to_vsync(vk::PresentModeKHR const mode) {
	switch (mode) {
	case vk::PresentModeKHR::eFifoRelaxed: return Vsync::Adaptive;
	case vk::PresentModeKHR::eMailbox: return Vsync::MultiBuffer;
	case vk::PresentModeKHR::eImmediate: return Vsync::Off;
	default:
	case vk::PresentModeKHR::eFifo: return Vsync::Strict;
	}
}

[[nodiscard]] constexpr auto to_mode(Vsync const vsync) {
	switch (vsync) {
	case Vsync::Adaptive: return vk::PresentModeKHR::eFifoRelaxed;
	case Vsync::MultiBuffer: return vk::PresentModeKHR::eMailbox;
	case Vsync::Off: return vk::PresentModeKHR::eImmediate;
	default:
	case Vsync::Strict: return vk::PresentModeKHR::eFifo;
	}
}

[[nodiscard]] constexpr auto glfw_platform_str(int const platform) -> std::string_view {
	switch (platform) {
	case GLFW_PLATFORM_NULL: return "Null";
	case GLFW_PLATFORM_WIN32: return "Win32";
	case GLFW_PLATFORM_X11: return "X11";
	case GLFW_PLATFORM_WAYLAND: return "Wayland";
	case GLFW_PLATFORM_COCOA: return "Cocoa";
	default: return "[unknown]";
	}
}

[[nodiscard]] auto create_default_shader(IResourceFactory const& factory) -> std::unique_ptr<IShader> {
	auto const vert_spirv = spirv::vert();
	auto const frag_spirv = spirv::frag();
	auto ret = factory.create_shader();
	if (!ret->load(vert_spirv, frag_spirv)) { throw Error{"Failed to create default shader"}; }
	return ret;
}

struct AssetLoaderBuilder {
	template <typename... Ts>
	auto build() {
		auto ret = AssetLoader{};
		(ret.add_loader(std::make_unique<Ts>(&data_loader, &resource_factory)), ...);
		return ret;
	}

	IDataLoader const& data_loader;
	IResourceFactory const& resource_factory;
};
} // namespace

void Context::OnDestroy::operator()(Context* ptr) const noexcept {
	if (!ptr) { return; }
	log.info("Context shutting down");
	ptr->wait_idle();
}

Context::Context(CreateInfo const& create_info)
	: m_window(std::make_unique<detail::RenderWindow>(create_info.platform_flags, create_info.window, create_info.render_device)),
	  m_resource_factory(std::make_unique<detail::ResourceFactory>(&m_window->get_render_device())) {

	auto default_shader = create_default_shader(get_resource_factory());
	m_resource_pool = std::make_unique<detail::ResourcePool>(&m_window->get_render_device(), std::move(default_shader));
	m_pass = create_render_pass(create_info.framebuffer_samples);
	m_renderer = m_pass->create_renderer();
	m_audio_mixer = std::make_unique<detail::AudioMixer>(create_info.sfx_buffers);

	auto const supported_modes = m_window->get_render_device().get_supported_present_modes();
	m_supported_vsync.reserve(supported_modes.size());
	for (auto const mode : supported_modes) { m_supported_vsync.push_back(to_vsync(mode)); }

	log.info("[{}] Context initialized, platform: {}", build_version_v, glfw_platform_str(glfwGetPlatform()));
	m_on_destroy.reset(this);
}

auto Context::framebuffer_size() const -> glm::ivec2 { return glm::vec2{swapchain_size()} * m_render_scale; }

auto Context::set_render_scale(float const scale) -> bool {
	if (scale < min_render_scale_v || scale > max_render_scale_v) { return false; }
	m_render_scale = scale;
	return true;
}

auto Context::get_vsync() const -> Vsync { return to_vsync(m_window->get_render_device().get_present_mode()); }

auto Context::set_vsync(Vsync const vsync) -> bool {
	if (vsync == get_vsync()) { return true; }
	auto const supported = get_supported_vsync();
	if (std::ranges::find(supported, vsync) == supported.end()) { return false; }
	m_window->get_render_device().set_present_mode(to_mode(vsync));
	return true;
}

auto Context::get_samples() const -> vk::SampleCountFlagBits { return m_requests.set_samples.value_or(m_pass->get_samples()); }

auto Context::get_supported_samples() const -> vk::SampleCountFlags {
	return m_window->get_render_device().get_gpu().properties.limits.framebufferColorSampleCounts;
}

auto Context::set_samples(vk::SampleCountFlagBits const samples) -> bool {
	if (samples == get_samples()) { return true; }
	if ((get_supported_samples() & samples) != samples) { return false; }
	m_requests.set_samples = samples;
	return true;
}

auto Context::next_frame() -> vk::CommandBuffer {
	m_cmd = m_window->next_frame();
	++m_fps.counter;
	process_requests();
	m_frame_start = kvf::Clock::now();
	return m_cmd;
}

auto Context::begin_render(kvf::Color const clear) -> IRenderer& {
	m_renderer->begin_render(m_cmd, framebuffer_size(), clear);
	return *m_renderer;
}

void Context::present() {
	m_renderer->end_render();
	auto const present_start = kvf::Clock::now();
	m_window->present(m_pass->get_render_target());
	m_cmd = vk::CommandBuffer{};
	update_stats(present_start);
}

void Context::wait_idle() {
	m_window->get_render_device().get_device().waitIdle();
	m_audio_mixer->stop_all();
}

auto Context::create_waiter() -> Waiter { return this; }

auto Context::create_render_pass(vk::SampleCountFlagBits const samples) const -> std::unique_ptr<IRenderPass> {
	return std::make_unique<detail::RenderPass>(&m_window->get_render_device(), m_resource_pool.get(), samples);
}

auto Context::create_asset_loader(gsl::not_null<IDataLoader const*> data_loader) const -> AssetLoader {
	auto builder = AssetLoaderBuilder{.data_loader = *data_loader, .resource_factory = *m_resource_factory};
	return builder
		.build<ShaderLoader, FontLoader, TextureLoader, TileSetLoader, TileSheetLoader, AudioBufferLoader, TransformAnimationLoader, FlipbookAnimationLoader>();
}

void Context::process_requests() {
	if (m_requests.is_empty()) { return; }

	m_window->get_render_device().get_device().waitIdle();
	if (m_requests.set_samples) {
		m_pass = create_render_pass(*m_requests.set_samples);
		m_requests.set_samples.reset();
	}
}

void Context::update_stats(kvf::Clock::time_point const present_start) {
	auto const now = kvf::Clock::now();
	m_frame_stats.present_dt = now - present_start;
	m_frame_stats.total_dt = now - m_frame_start;
	m_fps.elapsed += m_frame_stats.total_dt;
	if (m_fps.elapsed >= 1s) {
		m_fps.value = std::exchange(m_fps.counter, {});
		m_fps.elapsed = {};
	}
	m_frame_stats.framerate = m_fps.value == 0 ? m_fps.counter : m_fps.value;
	++m_frame_stats.total_frames;
	m_frame_stats.run_time = now - m_runtime_start;
}

void Context::Waiter::Deleter::operator()(Context* ptr) const {
	if (!ptr) { return; }
	ptr->wait_idle();
}
} // namespace le
