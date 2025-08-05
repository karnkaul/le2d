#pragma once
#include <klib/assert.hpp>
#include <klib/visitor.hpp>
#include <le2d/render_window.hpp>
#include <log.hpp>
#include <array>

namespace le::detail {
class RenderWindow : public IRenderWindow {
  public:
	explicit RenderWindow(PlatformFlag const platform_flags, WindowCreateInfo const& wci, kvf::RenderDeviceCreateInfo const& rdci = {})
		: m_window(create_window(platform_flags, wci)), m_render_device(m_window.get(), rdci) {}

  private:
	template <klib::EnumT E>
	static constexpr auto to_focus(int const f) {
		return f == GLFW_TRUE ? E::True : E::False;
	}

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice const& final { return m_render_device; }
	[[nodiscard]] auto get_window() const -> GLFWwindow* final { return m_window.get(); }

	[[nodiscard]] auto event_queue() const -> std::span<Event const> final { return m_event_queue; }

	auto next_frame() -> vk::CommandBuffer final {
		m_event_queue.clear();
		m_drops.clear();
		return m_render_device.next_frame();
	}

	void present(kvf::RenderTarget const& render_target) final { m_render_device.render(render_target); }

	[[nodiscard]] static auto self(GLFWwindow* window) -> RenderWindow& { return *static_cast<RenderWindow*>(glfwGetWindowUserPointer(window)); }

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
		if (glfwRawMouseMotionSupported() == GLFW_TRUE) { glfwSetInputMode(m_window.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); }
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

	kvf::UniqueWindow m_window;
	kvf::RenderDevice m_render_device;

	std::vector<Event> m_event_queue{};
	std::vector<std::string> m_drops{};
};
} // namespace le::detail
