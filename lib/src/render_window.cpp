#include <common.hpp>
#include <kvf/util.hpp>
#include <le2d/error.hpp>
#include <le2d/render_window.hpp>

namespace le {
RenderWindow::RenderWindow(glm::ivec2 const size, klib::CString const title) : m_window(create_window(size, title)), m_render_device(m_window.get()) {}

auto RenderWindow::framebuffer_size() const -> glm::ivec2 { return kvf::util::to_glm_vec<int>(m_render_device.get_framebuffer_extent()); }

auto RenderWindow::is_open() const -> bool { return glfwWindowShouldClose(m_window.get()) == GLFW_FALSE; }

void RenderWindow::set_closing() { glfwSetWindowShouldClose(m_window.get(), GLFW_TRUE); }

auto RenderWindow::next_frame() -> vk::CommandBuffer {
	m_event_queue.clear();
	return m_render_device.next_frame();
}

void RenderWindow::present(kvf::RenderTarget const& render_target) { m_render_device.render(render_target); }

auto RenderWindow::self(GLFWwindow* window) -> RenderWindow& { return *static_cast<RenderWindow*>(glfwGetWindowUserPointer(window)); }

auto RenderWindow::create_window(glm::ivec2 const size, klib::CString const title) -> kvf::UniqueWindow {
	auto ret = kvf::create_window(size, title);
	if (!ret) { throw Error{"Failed to create Window"}; }

	glfwSetWindowUserPointer(ret.get(), this);
	set_glfw_callbacks(ret.get());
	return ret;
}

void RenderWindow::set_glfw_callbacks(GLFWwindow* window) {
	using namespace event;
	static auto const push_event = [](GLFWwindow* window, Event const& event) { self(window).m_event_queue.push_back(event); };
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
		push_event(window, Key{.key = key, .action = action, .mods = mods});
	});
	glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int in_focus) { push_event(window, event::Focus{.in_focus = in_focus == GLFW_TRUE}); });
}
} // namespace le
