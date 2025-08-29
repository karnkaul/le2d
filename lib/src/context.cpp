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

class ContextImpl : public IContext {
  public:
	class Waiter;

	using SpirV = std::span<std::uint32_t const>;
	using CreateInfo = ContextCreateInfo;

	static constexpr auto min_render_scale_v{0.2f};
	static constexpr auto max_render_scale_v{8.0f};

	explicit ContextImpl(CreateInfo const& create_info = {})
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

  private:
	[[nodiscard]] auto get_render_window() const -> IRenderWindow const& final { return *m_window; }
	[[nodiscard]] auto get_resource_factory() const -> IResourceFactory const& final { return *m_resource_factory; }
	[[nodiscard]] auto get_audio_mixer() const -> IAudioMixer& final { return *m_audio_mixer; }
	[[nodiscard]] auto get_default_shader() const -> IShader const& final { return m_resource_pool->get_default_shader(); }

	[[nodiscard]] auto swapchain_size() const -> glm::ivec2 final { return m_window->framebuffer_size(); }
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2 final { return glm::vec2{swapchain_size()} * m_render_scale; }
	[[nodiscard]] auto event_queue() const -> std::span<Event const> final { return m_window->event_queue(); }

	[[nodiscard]] auto is_running() const -> bool final { return m_window->is_open(); }
	void shutdown() final { m_window->set_closing(); }
	void cancel_window_close() final { m_window->cancel_close(); }

	[[nodiscard]] auto get_render_scale() const -> float final { return m_render_scale; }
	auto set_render_scale(float scale) -> bool final {
		if (scale < min_render_scale_v || scale > max_render_scale_v) { return false; }
		m_render_scale = scale;
		return true;
	}

	[[nodiscard]] auto get_supported_vsync() const -> std::span<Vsync const> final { return m_supported_vsync; }
	[[nodiscard]] auto get_vsync() const -> Vsync final { return to_vsync(m_window->get_render_device().get_present_mode()); }
	auto set_vsync(Vsync vsync) -> bool final {
		if (vsync == get_vsync()) { return true; }
		auto const supported = get_supported_vsync();
		if (std::ranges::find(supported, vsync) == supported.end()) { return false; }
		m_window->get_render_device().set_present_mode(to_mode(vsync));
		return true;
	}

	auto set_fullscreen(GLFWmonitor* target = nullptr) -> bool final { return m_window->set_fullscreen(target); }
	void set_windowed(glm::ivec2 const size = {1280, 720}) final { m_window->set_windowed(size); }
	void set_visible(bool const visible) final { m_window->set_visible(visible); }

	/// \returns Current MSAA samples.
	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits final { return m_requests.set_samples.value_or(m_pass->get_samples()); }
	/// \returns Supported MSAA samples.
	[[nodiscard]] auto get_supported_samples() const -> vk::SampleCountFlags final {
		return m_window->get_render_device().get_gpu().properties.limits.framebufferColorSampleCounts;
	}
	auto set_samples(vk::SampleCountFlagBits samples) -> bool final {
		if (samples == get_samples()) { return true; }
		if ((get_supported_samples() & samples) != samples) { return false; }
		m_requests.set_samples = samples;
		return true;
	}

	auto next_frame() -> vk::CommandBuffer final {
		m_cmd = m_window->next_frame();
		++m_fps.counter;
		process_requests();
		m_frame_start = kvf::Clock::now();
		return m_cmd;
	}
	[[nodiscard]] auto begin_render(kvf::Color clear = kvf::black_v) -> IRenderer& final {
		m_renderer->begin_render(m_cmd, framebuffer_size(), clear);
		return *m_renderer;
	}
	void present() final {
		m_renderer->end_render();
		auto const present_start = kvf::Clock::now();
		m_window->present(m_pass->get_render_target());
		m_cmd = vk::CommandBuffer{};
		update_stats(present_start);
	}

	void wait_idle() final {
		m_window->get_render_device().get_device().waitIdle();
		m_audio_mixer->stop_all();
	}

	[[nodiscard]] auto get_frame_stats() const -> FrameStats const& final { return m_frame_stats; }

	[[nodiscard]] auto create_render_pass(vk::SampleCountFlagBits samples) const -> std::unique_ptr<IRenderPass> final {
		return std::make_unique<detail::RenderPass>(&m_window->get_render_device(), m_resource_pool.get(), samples);
	}
	[[nodiscard]] auto create_asset_loader(gsl::not_null<IDataLoader const*> data_loader) const -> AssetLoader final {
		auto builder = AssetLoaderBuilder{.data_loader = *data_loader, .resource_factory = *m_resource_factory};
		return builder.build<ShaderLoader, FontLoader, TextureLoader, TileSetLoader, TileSheetLoader, AudioBufferLoader, TransformAnimationLoader,
							 FlipbookAnimationLoader>();
	}

	struct OnDestroy {
		void operator()(IContext* ptr) const noexcept {
			if (!ptr) { return; }
			log.info("Context shutting down");
			ptr->wait_idle();
		}
	};

	struct Fps {
		std::int32_t counter{};
		std::int32_t value{};
		kvf::Seconds elapsed{};
	};

	struct Requests {
		[[nodiscard]] auto is_empty() const -> bool { return !set_samples; }

		std::optional<vk::SampleCountFlagBits> set_samples{};
	};

	void process_requests() {
		if (m_requests.is_empty()) { return; }

		m_window->get_render_device().get_device().waitIdle();
		if (m_requests.set_samples) {
			m_pass = create_render_pass(*m_requests.set_samples);
			m_requests.set_samples.reset();
		}
	}

	void update_stats(kvf::Clock::time_point present_start) {
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

	std::unique_ptr<IRenderWindow> m_window{};
	std::unique_ptr<IRenderPass> m_pass{};
	std::unique_ptr<IRenderer> m_renderer{};
	std::vector<Vsync> m_supported_vsync{};

	std::unique_ptr<IResourceFactory> m_resource_factory{};
	std::unique_ptr<IResourcePool> m_resource_pool{};
	std::unique_ptr<IAudioMixer> m_audio_mixer{};

	Requests m_requests{};

	float m_render_scale{1.0f};

	vk::CommandBuffer m_cmd{};

	kvf::Clock::time_point m_frame_start{};
	kvf::Clock::time_point m_runtime_start{kvf::Clock::now()};
	Fps m_fps{};
	FrameStats m_frame_stats{};

	std::unique_ptr<IContext, OnDestroy> m_on_destroy{};
};
} // namespace

auto IContext::create(CreateInfo const& create_info) -> std::unique_ptr<IContext> { return std::make_unique<ContextImpl>(create_info); }

auto IContext::create_waiter() -> Waiter { return Waiter{this}; }
} // namespace le
