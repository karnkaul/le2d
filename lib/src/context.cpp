#include "le2d/context.hpp"
#include "le2d/asset/asset_type_loaders.hpp"
#include "le2d/error.hpp"
#include <capo/engine.hpp>
#include <detail/audio_mixer.hpp>
#include <detail/pipeline_pool.hpp>
#include <detail/render_pass.hpp>
#include <detail/resource/resource_factory.hpp>
#include <detail/resource/resource_pool.hpp>
#include <klib/assert.hpp>
#include <klib/version_str.hpp>
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

template <klib::EnumT E>
constexpr auto to_focus(int const f) {
	return f == GLFW_TRUE ? E::True : E::False;
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

struct Display {
	gsl::not_null<GLFWmonitor*> monitor;
	gsl::not_null<GLFWvidmode const*> video_mode;
};

constexpr auto to_display_ratio(glm::vec2 const w_size, glm::vec2 const fb_size) -> glm::vec2 {
	if (!kvf::is_positive(w_size) || !kvf::is_positive(fb_size)) { return {}; }
	return fb_size / w_size;
}

auto window_display(GLFWwindow* window) -> std::optional<Display> {
	auto* monitor = glfwGetWindowMonitor(window);
	if (monitor == nullptr) { return {}; }
	auto const* video_mode = glfwGetVideoMode(monitor);
	if (video_mode == nullptr) { return {}; }
	return Display{.monitor = monitor, .video_mode = video_mode};
}

auto fastest_display() {
	auto count = int{};
	auto const* p_monitors = glfwGetMonitors(&count);
	auto const monitors = std::span{p_monitors, std::size_t(count)};
	auto ret = std::optional<Display>{};
	for (auto* monitor : monitors) {
		auto const* video_mode = glfwGetVideoMode(monitor);
		if (video_mode == nullptr) { continue; }
		if (!ret || video_mode->refreshRate > ret->video_mode->refreshRate) { ret = Display{.monitor = monitor, .video_mode = video_mode}; }
	}
	return ret;
}

auto target_display(GLFWmonitor* desired) -> std::optional<Display> {
	GLFWvidmode const* video_mode{};
	if (desired == nullptr) {
		auto const fastest_monitor = fastest_display();
		if (!fastest_monitor) { return {}; }
		desired = fastest_monitor->monitor;
		video_mode = fastest_monitor->video_mode;
	}
	if (video_mode == nullptr) {
		video_mode = glfwGetVideoMode(desired);
		if (video_mode == nullptr) { return {}; }
	}
	return Display{.monitor = desired, .video_mode = video_mode};
}

template <typename F>
auto get_glfw_vec2(GLFWwindow* window, F glfw_func) -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfw_func(window, &ret.x, &ret.y);
	return ret;
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

class ContextImpl : public Context {
  public:
	class Waiter;

	using SpirV = std::span<std::uint32_t const>;
	using CreateInfo = ContextCreateInfo;

	static constexpr auto min_render_scale_v{0.2f};
	static constexpr auto max_render_scale_v{8.0f};

	explicit ContextImpl(CreateInfo const& create_info = {})
		: m_window(create_window(create_info.platform_flags, create_info.window)),
		  m_render_device(std::make_unique<kvf::RenderDevice>(get_window(), create_info.render_device)),
		  m_resource_factory(std::make_unique<detail::ResourceFactory>(m_render_device.get())) {

		auto default_shader = create_default_shader(get_resource_factory());
		m_resource_pool = std::make_unique<detail::ResourcePool>(m_render_device.get(), std::move(default_shader));
		m_pass = create_render_pass(create_info.framebuffer_samples);
		m_renderer = m_pass->create_renderer();
		m_audio_mixer = std::make_unique<detail::AudioMixer>(create_info.sfx_buffers);

		auto const supported_modes = m_render_device->get_supported_present_modes();
		m_supported_vsync.reserve(supported_modes.size());
		for (auto const mode : supported_modes) { m_supported_vsync.push_back(to_vsync(mode)); }

		log.info("[{}] Context initialized, platform: {}", build_version_v, glfw_platform_str(glfwGetPlatform()));
		m_on_destroy.reset(this);
	}

  private:
	struct OnDestroy {
		void operator()(Context* ptr) const noexcept {
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

	static constexpr auto to_display_ratio(glm::vec2 const w_size, glm::vec2 const fb_size) -> glm::vec2 {
		if (!kvf::is_positive(w_size) || !kvf::is_positive(fb_size)) { return {}; }
		return fb_size / w_size;
	}

	[[nodiscard]] auto get_window() const -> GLFWwindow* final { return m_window.get(); }
	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice const& final { return *m_render_device; }
	[[nodiscard]] auto get_resource_factory() const -> IResourceFactory const& final { return *m_resource_factory; }
	[[nodiscard]] auto get_audio_mixer() const -> IAudioMixer& final { return *m_audio_mixer; }
	[[nodiscard]] auto get_default_shader() const -> IShader const& final { return m_resource_pool->get_default_shader(); }
	[[nodiscard]] auto get_renderer() const -> IRenderer const& final { return *m_renderer; }

	[[nodiscard]] auto get_render_scale() const -> float final { return m_render_scale; }
	auto set_render_scale(float scale) -> bool final {
		if (scale < min_render_scale_v || scale > max_render_scale_v) { return false; }
		m_render_scale = scale;
		return true;
	}

	[[nodiscard]] auto get_supported_vsync() const -> std::span<Vsync const> final { return m_supported_vsync; }
	[[nodiscard]] auto get_vsync() const -> Vsync final { return to_vsync(m_render_device->get_present_mode()); }
	auto set_vsync(Vsync vsync) -> bool final {
		if (vsync == get_vsync()) { return true; }
		auto const supported = get_supported_vsync();
		if (std::ranges::find(supported, vsync) == supported.end()) { return false; }
		m_render_device->set_present_mode(to_mode(vsync));
		return true;
	}

	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits final { return m_requests.set_samples.value_or(m_pass->get_samples()); }
	[[nodiscard]] auto get_supported_samples() const -> vk::SampleCountFlags final {
		return m_render_device->get_gpu().properties.limits.framebufferColorSampleCounts;
	}
	auto set_samples(vk::SampleCountFlagBits const samples) -> bool final {
		if (samples == get_samples()) { return true; }
		if ((get_supported_samples() & samples) != samples) { return false; }
		m_requests.set_samples = samples;
		return true;
	}

	auto next_frame() -> vk::CommandBuffer final {
		m_event_queue.clear();
		m_drops.clear();
		m_cmd = m_render_device->next_frame();
		process_requests();
		update_timings_and_stats(kvf::Clock::now());
		return m_cmd;
	}
	[[nodiscard]] auto event_queue() const -> std::span<Event const> final { return m_event_queue; }
	[[nodiscard]] auto begin_render(kvf::Color clear = kvf::black_v) -> IRenderer& final {
		m_renderer->begin_render(m_cmd, main_pass_size(), clear);
		return *m_renderer;
	}
	void present() final {
		m_renderer->end_render();
		m_render_device->render(m_pass->get_render_target());
		m_cmd = vk::CommandBuffer{};
		m_frame_finish = kvf::Clock::now();
	}

	[[nodiscard]] auto get_frame_stats() const -> FrameStats const& final { return m_frame_stats; }

	[[nodiscard]] auto create_render_pass(vk::SampleCountFlagBits samples) const -> std::unique_ptr<IRenderPass> final {
		return std::make_unique<detail::RenderPass>(m_render_device.get(), m_resource_pool.get(), samples);
	}
	[[nodiscard]] auto create_asset_loader(gsl::not_null<IDataLoader const*> data_loader) const -> AssetLoader final {
		auto builder = AssetLoaderBuilder{.data_loader = *data_loader, .resource_factory = *m_resource_factory};
		return builder.build<ShaderLoader, FontLoader, TextureLoader, TileSetLoader, TileSheetLoader, AudioBufferLoader, TransformAnimationLoader,
							 FlipbookAnimationLoader>();
	}

	void process_requests() {
		if (m_requests.is_empty()) { return; }

		m_render_device->get_device().waitIdle();
		if (m_requests.set_samples) {
			m_pass->recreate(*m_requests.set_samples);
			m_requests.set_samples.reset();
		}
	}

	void update_timings_and_stats(kvf::Clock::time_point const now) {
		m_frame_stats.total_dt = now - m_frame_start;
		m_frame_stats.frame_dt = m_frame_finish - m_frame_start;
		m_frame_start = now;
		++m_fps.counter;
		m_fps.elapsed += m_frame_stats.total_dt;
		if (m_fps.elapsed >= 1s) {
			m_fps.value = std::exchange(m_fps.counter, {});
			m_fps.elapsed = {};
		}
		m_frame_stats.framerate = m_fps.value == 0 ? m_fps.counter : m_fps.value;
		++m_frame_stats.total_frames;
		m_frame_stats.run_time = now - m_runtime_start;
	}

	[[nodiscard]] static auto self(GLFWwindow* window) -> ContextImpl& { return *static_cast<ContextImpl*>(glfwGetWindowUserPointer(window)); }

	[[nodiscard]] auto create_window(PlatformFlag const platform_flags, WindowCreateInfo const& wci) -> kvf::UniqueWindow {
		set_platform_flags(platform_flags);
		auto const visitor = klib::Visitor{
			[](WindowInfo const& wi) {
				using Hint = kvf::WindowHint;
				auto const hints = std::array{
					Hint{.hint = GLFW_RESIZABLE, .value = (wi.flags & WindowFlag::Resizeable) == WindowFlag::Resizeable ? GLFW_TRUE : GLFW_FALSE},
					Hint{.hint = GLFW_DECORATED, .value = (wi.flags & WindowFlag::Decorated) == WindowFlag::Decorated ? GLFW_TRUE : GLFW_FALSE},
					Hint{.hint = GLFW_VISIBLE, .value = (wi.flags & WindowFlag::Visible) == WindowFlag::Visible ? GLFW_TRUE : GLFW_FALSE},
					Hint{.hint = GLFW_MAXIMIZED, .value = (wi.flags & WindowFlag::Maximized) == WindowFlag::Maximized ? GLFW_TRUE : GLFW_FALSE},
					Hint{.hint = GLFW_FOCUS_ON_SHOW, .value = (wi.flags & WindowFlag::FocusOnShow) == WindowFlag::FocusOnShow ? GLFW_TRUE : GLFW_FALSE},
					Hint{.hint = GLFW_SCALE_TO_MONITOR,
						 .value = (wi.flags & WindowFlag::ScaleToMonitor) == WindowFlag::ScaleToMonitor ? GLFW_TRUE : GLFW_FALSE},
					Hint{.hint = GLFW_SCALE_FRAMEBUFFER,
						 .value = (wi.flags & WindowFlag::ScaleFramebuffer) == WindowFlag::ScaleFramebuffer ? GLFW_TRUE : GLFW_FALSE},
				};
				return kvf::create_window(wi.size, wi.title, hints);
			},
			[](FullscreenInfo const& fi) { return kvf::create_fullscreen_window(fi.title); },
		};
		auto ret = std::visit(visitor, wci);
		KLIB_ASSERT(ret);
		glfwSetWindowUserPointer(ret.get(), this);
		set_glfw_callbacks(ret.get());
		if (glfwRawMouseMotionSupported() == GLFW_TRUE) { glfwSetInputMode(get_window(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); }
		return ret;
	}

	static void set_platform_flags(PlatformFlag const flags) {
		auto forcing_x11 = false;
		if ((flags & PlatformFlag::ForceX11) == PlatformFlag::ForceX11 && glfwPlatformSupported(GLFW_PLATFORM_X11)) {
			glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
			forcing_x11 = true;
			log.info("-- Forcing X11");
		}
		if (!forcing_x11 && (flags & PlatformFlag::NoLibdecor) == PlatformFlag::NoLibdecor && glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)) {
			glfwInitHint(GLFW_WAYLAND_LIBDECOR, GLFW_WAYLAND_DISABLE_LIBDECOR);
			log.info("-- Disabling libdecor");
		}
	}

	static void set_glfw_callbacks(GLFWwindow* window) {
		static auto const push_event = [](GLFWwindow* window, Event const& event) { self(window).m_event_queue.push_back(event); };
		glfwSetWindowCloseCallback(window, [](GLFWwindow* window) { push_event(window, event::WindowClose{}); });
		glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int in_focus) { push_event(window, to_focus<event::WindowFocus>(in_focus)); });
		glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered) { push_event(window, to_focus<event::CursorFocus>(entered)); });
		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
			push_event(window, event::Key{.key = key, .action = action, .mods = mods});
		});
		glfwSetCharCallback(window, [](GLFWwindow* window, std::uint32_t const codepoint) { push_event(window, event::Codepoint{codepoint}); });
		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int x, int y) { push_event(window, event::WindowResize{x, y}); });
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int x, int y) { push_event(window, event::FramebufferResize{x, y}); });
		glfwSetWindowPosCallback(window, [](GLFWwindow* window, int x, int y) { push_event(window, event::WindowPos{x, y}); });
		glfwSetWindowIconifyCallback(window, [](GLFWwindow* window, int i) { push_event(window, to_focus<event::WindowIconify>(i)); });
		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) { self(window).on_cursor_pos({x, y}); });
		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
			push_event(window, event::MouseButton{.button = button, .action = action, .mods = mods});
		});
		glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) { push_event(window, event::Scroll{x, y}); });
		glfwSetDropCallback(window, [](GLFWwindow* window, int const count, char const** paths) { self(window).on_drop(count, paths); });
	}

	void on_cursor_pos(window::vec2 pos) {
		auto const event = event::CursorPos{
			.window = pos,
			.normalized = pos.to_ndc(window_size()),
		};
		m_event_queue.emplace_back(event);
	}

	void on_drop(int count, char const** paths) {
		auto const span = std::span{paths, std::size_t(count)};
		m_drops = {span.begin(), span.end()};
		m_event_queue.emplace_back(event::Drop{.paths = m_drops});
	}

	kvf::UniqueWindow m_window{};
	std::unique_ptr<kvf::RenderDevice> m_render_device{};

	std::unique_ptr<IRenderPass> m_pass{};
	std::unique_ptr<IRenderer> m_renderer{};
	std::vector<Vsync> m_supported_vsync{};

	std::unique_ptr<IResourceFactory> m_resource_factory{};
	std::unique_ptr<detail::ResourcePool> m_resource_pool{};
	std::unique_ptr<IAudioMixer> m_audio_mixer{};

	std::vector<Event> m_event_queue{};
	std::vector<std::string> m_drops{};

	Requests m_requests{};

	float m_render_scale{1.0f};

	vk::CommandBuffer m_cmd{};

	kvf::Clock::time_point m_frame_start{};
	kvf::Clock::time_point m_frame_finish{};
	kvf::Clock::time_point m_runtime_start{kvf::Clock::now()};
	Fps m_fps{};
	FrameStats m_frame_stats{};

	std::unique_ptr<Context, OnDestroy> m_on_destroy{};
};
} // namespace

auto Context::create(CreateInfo const& create_info) -> std::unique_ptr<Context> { return std::make_unique<ContextImpl>(create_info); }

auto Context::window_size() const -> glm::ivec2 { return get_glfw_vec2(get_window(), &glfwGetWindowSize); }
auto Context::framebuffer_size() const -> glm::ivec2 { return get_glfw_vec2(get_window(), &glfwGetFramebufferSize); }
auto Context::swapchain_extent() const -> vk::Extent2D { return get_render_device().get_framebuffer_extent(); }
auto Context::main_pass_size() const -> glm::ivec2 { return glm::vec2{framebuffer_size()} * get_render_scale(); }
auto Context::display_ratio() const -> glm::vec2 { return to_display_ratio(window_size(), framebuffer_size()); }

auto Context::get_title() const -> klib::CString { return glfwGetWindowTitle(get_window()); }

// NOLINTNEXTLINE(readability-make-member-function-const)
void Context::set_title(klib::CString const title) { glfwSetWindowTitle(get_window(), title.c_str()); }

auto Context::get_refresh_rate() const -> std::int32_t {
	if (auto const display = window_display(get_window())) { return display->video_mode->refreshRate; }
	if (auto const display = fastest_display()) { return display->video_mode->refreshRate; }
	return 0;
}

auto Context::is_fullscreen() const -> bool { return glfwGetWindowMonitor(get_window()) != nullptr; }

auto Context::set_fullscreen(GLFWmonitor* target) -> bool {
	set_visible(true);
	auto const display = target_display(target);
	if (!display) { return false; }
	auto const* vm = display->video_mode.get();
	glfwSetWindowMonitor(get_window(), display->monitor, 0, 0, vm->width, vm->height, vm->refreshRate);
	return true;
}

void Context::set_windowed(glm::ivec2 const size) {
	set_visible(true);
	if (!kvf::is_positive(size)) { return; }
	if (is_fullscreen()) {
		glfwSetWindowMonitor(get_window(), nullptr, 0, 0, size.x, size.y, 0);
	} else {
		glfwSetWindowSize(get_window(), size.x, size.y);
	}
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Context::set_visible(bool const visible) {
	if (visible) {
		glfwShowWindow(get_window());
	} else {
		glfwHideWindow(get_window());
	}
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Context::lock_aspect_ratio(bool const lock) {
	auto const size = lock ? window_size() : glm::ivec2{GLFW_DONT_CARE, GLFW_DONT_CARE};
	glfwSetWindowAspectRatio(get_window(), size.x, size.y);
}

auto Context::is_running() const -> bool { return glfwWindowShouldClose(get_window()) == GLFW_FALSE; }
// NOLINTNEXTLINE(readability-make-member-function-const)
void Context::set_window_close() { glfwSetWindowShouldClose(get_window(), GLFW_TRUE); }
// NOLINTNEXTLINE(readability-make-member-function-const)
void Context::cancel_window_close() { glfwSetWindowShouldClose(get_window(), GLFW_FALSE); }

// NOLINTNEXTLINE(readability-make-member-function-const)
void Context::wait_idle() {
	get_render_device().get_device().waitIdle();
	get_audio_mixer().stop_all();
}

auto Context::create_waiter() -> Waiter { return Waiter{this}; }
} // namespace le
