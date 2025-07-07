#include <capo/engine.hpp>
#include <detail/audio_mixer.hpp>
#include <detail/pipeline_pool.hpp>
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
constexpr auto to_vsync(vk::PresentModeKHR const mode) {
	switch (mode) {
	case vk::PresentModeKHR::eFifoRelaxed: return Vsync::Adaptive;
	case vk::PresentModeKHR::eMailbox: return Vsync::MultiBuffer;
	case vk::PresentModeKHR::eImmediate: return Vsync::Off;
	default:
	case vk::PresentModeKHR::eFifo: return Vsync::Strict;
	}
}

constexpr auto to_mode(Vsync const vsync) {
	switch (vsync) {
	case Vsync::Adaptive: return vk::PresentModeKHR::eFifoRelaxed;
	case Vsync::MultiBuffer: return vk::PresentModeKHR::eMailbox;
	case Vsync::Off: return vk::PresentModeKHR::eImmediate;
	default:
	case Vsync::Strict: return vk::PresentModeKHR::eFifo;
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
	: m_window(std::make_unique<detail::RenderWindow>(create_info.window, create_info.render_device)),
	  m_pass(&m_window->get_render_device(), create_info.framebuffer_samples),
	  m_resource_factory(std::make_unique<detail::ResourceFactory>(&m_window->get_render_device())) {

	auto default_shader = create_default_shader(get_resource_factory());
	m_resource_pool = std::make_unique<detail::ResourcePool>(&m_window->get_render_device(), std::move(default_shader));
	m_audio_mixer = std::make_unique<detail::AudioMixer>(create_info.sfx_buffers);

	auto const supported_modes = m_window->get_render_device().get_supported_present_modes();
	m_supported_vsync.reserve(supported_modes.size());
	for (auto const mode : supported_modes) { m_supported_vsync.push_back(to_vsync(mode)); }

	log.info("[{}] Context initialized", build_version_v);
	m_on_destroy.reset(this);
}

auto Context::get_latest_gamepad() -> Gamepad const& {
	if (!m_latest_gamepad.is_connected()) {
		m_latest_gamepad = Gamepad::get_active();
	} else {
		m_latest_gamepad = Gamepad::get_by_id(m_latest_gamepad.get_id());
	}
	return m_latest_gamepad;
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
	return m_window->get_render_device().set_present_mode(to_mode(vsync));
}

auto Context::next_frame() -> vk::CommandBuffer {
	m_cmd = m_window->next_frame();
	++m_fps.counter;
	m_frame_start = kvf::Clock::now();
	return m_cmd;
}

auto Context::begin_render(kvf::Color const clear) -> Renderer {
	if (!m_cmd) { return {}; }
	m_pass.set_clear_color(clear);
	return m_pass.begin_render(*m_resource_pool, m_cmd, framebuffer_size());
}

void Context::present() {
	auto const present_start = kvf::Clock::now();
	m_window->present(m_pass.get_render_target());
	m_cmd = vk::CommandBuffer{};
	update_stats(present_start);
}

void Context::wait_idle() {
	m_window->get_render_device().get_device().waitIdle();
	m_audio_mixer->stop_all();
}

auto Context::create_waiter() -> Waiter { return this; }

auto Context::create_render_pass(vk::SampleCountFlagBits const samples) const -> RenderPass { return RenderPass{&m_pass.get_render_device(), samples}; }

auto Context::create_asset_loader(gsl::not_null<IDataLoader const*> data_loader) const -> AssetLoader {
	auto builder = AssetLoaderBuilder{.data_loader = *data_loader, .resource_factory = *m_resource_factory};
	return builder
		.build<ShaderLoader, FontLoader, TextureLoader, TileSetLoader, TileSheetLoader, AudioBufferLoader, TransformAnimationLoader, FlipbookAnimationLoader>();
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
