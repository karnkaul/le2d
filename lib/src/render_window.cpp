#include <common.hpp>
#include <klib/assert.hpp>
#include <klib/visitor.hpp>
#include <kvf/is_positive.hpp>
#include <kvf/util.hpp>
#include <le2d/error.hpp>
#include <le2d/render_window.hpp>
#include <span>

namespace le {
namespace {
template <klib::EnumT E>
constexpr auto to_focus(int const f) {
	return f == GLFW_TRUE ? E::True : E::False;
}

constexpr auto to_display_ratio(glm::vec2 const w_size, glm::vec2 const fb_size) -> glm::vec2 {
	if (!kvf::is_positive(w_size) || !kvf::is_positive(fb_size)) { return {}; }
	return fb_size / w_size;
}
} // namespace

RenderWindow::RenderWindow(WindowCreateInfo const& wci, kvf::RenderDeviceCreateInfo const& rdci)
	: m_window(create_window(wci)), m_render_device(m_window.get(), rdci) {}

auto RenderWindow::window_size() const -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfwGetWindowSize(m_window.get(), &ret.x, &ret.y);
	return ret;
}

auto RenderWindow::framebuffer_size() const -> glm::ivec2 { return kvf::util::to_glm_vec<int>(m_render_device.get_framebuffer_extent()); }

auto RenderWindow::is_open() const -> bool { return glfwWindowShouldClose(m_window.get()) == GLFW_FALSE; }

void RenderWindow::set_closing() { glfwSetWindowShouldClose(m_window.get(), GLFW_TRUE); }

auto RenderWindow::display_ratio() const -> glm::vec2 { return to_display_ratio(window_size(), framebuffer_size()); }

auto RenderWindow::next_frame() -> vk::CommandBuffer {
	m_event_queue.clear();
	m_drops.clear();
	return m_render_device.next_frame();
}

void RenderWindow::present(kvf::RenderTarget const& render_target) { m_render_device.render(render_target); }

auto RenderWindow::self(GLFWwindow* window) -> RenderWindow& { return *static_cast<RenderWindow*>(glfwGetWindowUserPointer(window)); }

auto RenderWindow::create_window(WindowCreateInfo const& wci) -> kvf::UniqueWindow {
	auto const visitor = klib::Visitor{
		[](WindowInfo const& wi) { return kvf::create_window(wi.size, wi.title, wi.decorated); },
		[](FullscreenInfo const& fi) { return kvf::create_fullscreen_window(fi.title, fi.target); },
	};
	auto ret = std::visit(visitor, wci);
	KLIB_ASSERT(ret);
	glfwSetWindowUserPointer(ret.get(), this);
	set_glfw_callbacks(ret.get());
	if (glfwRawMouseMotionSupported() == GLFW_TRUE) { glfwSetInputMode(m_window.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); }
	return ret;
}

void RenderWindow::set_glfw_callbacks(GLFWwindow* window) {
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
}

void RenderWindow::on_cursor_pos(window::vec2 const pos) {
	auto const event = event::CursorPos{
		.window = pos,
		.normalized = pos.to_ndc(window_size()),
	};
	m_event_queue.emplace_back(event);
}

void RenderWindow::on_drop(int const count, char const** paths) {
	auto const span = std::span{paths, std::size_t(count)};
	m_drops = {span.begin(), span.end()};
	m_event_queue.emplace_back(event::Drop{.paths = m_drops});
}
} // namespace le
