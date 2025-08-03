#include <klib/assert.hpp>
#include <klib/visitor.hpp>
#include <kvf/is_positive.hpp>
#include <kvf/util.hpp>
#include <le2d/error.hpp>
#include <le2d/render_window.hpp>
#include <optional>

namespace le {
namespace {
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
} // namespace

auto IRenderWindow::window_size() const -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfwGetWindowSize(get_window(), &ret.x, &ret.y);
	return ret;
}

auto IRenderWindow::framebuffer_size() const -> glm::ivec2 {
	auto ret = glm::ivec2{};
	glfwGetFramebufferSize(get_window(), &ret.x, &ret.y);
	return ret;
}

auto IRenderWindow::is_open() const -> bool { return glfwWindowShouldClose(get_window()) == GLFW_FALSE; }

// NOLINTNEXTLINE(readability-make-member-function-const)
void IRenderWindow::set_closing() { glfwSetWindowShouldClose(get_window(), GLFW_TRUE); }

// NOLINTNEXTLINE(readability-make-member-function-const)
void IRenderWindow::cancel_close() { glfwSetWindowShouldClose(get_window(), GLFW_FALSE); }

auto IRenderWindow::display_ratio() const -> glm::vec2 { return to_display_ratio(window_size(), framebuffer_size()); }

auto IRenderWindow::get_title() const -> klib::CString { return glfwGetWindowTitle(get_window()); }

void IRenderWindow::set_title(klib::CString const title) const { glfwSetWindowTitle(get_window(), title.c_str()); }

auto IRenderWindow::get_refresh_rate() const -> std::int32_t {
	if (auto const display = window_display(get_window())) { return display->video_mode->refreshRate; }
	if (auto const display = fastest_display()) { return display->video_mode->refreshRate; }
	return 0;
}

auto IRenderWindow::is_fullscreen() const -> bool { return glfwGetWindowMonitor(get_window()) != nullptr; }

// NOLINTNEXTLINE(readability-make-member-function-const)
auto IRenderWindow::set_fullscreen(GLFWmonitor* target) -> bool {
	set_visible(true);
	auto const display = target_display(target);
	if (!display) { return false; }
	auto const* vm = display->video_mode.get();
	glfwSetWindowMonitor(get_window(), display->monitor, 0, 0, vm->width, vm->height, vm->refreshRate);
	return true;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
auto IRenderWindow::set_windowed(glm::ivec2 const size) -> bool {
	set_visible(true);
	if (!kvf::is_positive(size)) { return false; }
	if (is_fullscreen()) {
		glfwSetWindowMonitor(get_window(), nullptr, 0, 0, size.x, size.y, 0);
	} else {
		glfwSetWindowSize(get_window(), size.x, size.y);
	}
	return true;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void IRenderWindow::set_visible(bool const visible) {
	if (visible) {
		glfwShowWindow(get_window());
	} else {
		glfwHideWindow(get_window());
	}
}

auto IRenderWindow::get_render_device() -> kvf::RenderDevice& {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	return const_cast<kvf::RenderDevice&>(std::as_const(*this).get_render_device());
}
} // namespace le
