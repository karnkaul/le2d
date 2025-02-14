#pragma once
#include <glm/vec2.hpp>
#include <klib/c_string.hpp>
#include <kvf/device_block.hpp>
#include <kvf/render_device.hpp>
#include <kvf/window.hpp>
#include <le2d/event.hpp>

namespace le {
class RenderWindow {
  public:
	explicit RenderWindow(glm::ivec2 size, klib::CString title, bool decorated = true);
	explicit RenderWindow(klib::CString title, GLFWmonitor* target = nullptr);

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice const& { return m_render_device; }
	[[nodiscard]] auto get_render_device() -> kvf::RenderDevice& { return m_render_device; }

	[[nodiscard]] auto window_size() const -> glm::ivec2;
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;

	[[nodiscard]] auto is_open() const -> bool;
	void set_closing();

	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_event_queue; }

	[[nodiscard]] auto display_ratio() const -> glm::vec2;

	auto next_frame() -> vk::CommandBuffer;
	void present(kvf::RenderTarget const& render_target);

  private:
	void setup();

	[[nodiscard]] static auto self(GLFWwindow* window) -> RenderWindow&;

	[[nodiscard]] auto create_window(glm::ivec2 size, klib::CString title, bool decorated) -> kvf::UniqueWindow;
	[[nodiscard]] auto create_window(klib::CString title, GLFWmonitor* target) -> kvf::UniqueWindow;

	static void set_glfw_callbacks(GLFWwindow* window);

	void on_cursor_pos(window::vec2 pos);
	void on_drop(int count, char const** paths);

	kvf::UniqueWindow m_window;
	kvf::RenderDevice m_render_device;

	std::vector<Event> m_event_queue{};
	std::vector<std::string> m_drops{};
};
} // namespace le
