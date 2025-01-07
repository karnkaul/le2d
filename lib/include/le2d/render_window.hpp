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
	explicit RenderWindow(glm::ivec2 size, klib::CString title);

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice const& { return m_render_device; }
	[[nodiscard]] auto get_render_device() -> kvf::RenderDevice& { return m_render_device; }

	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;

	[[nodiscard]] auto is_open() const -> bool;
	void set_closing();

	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_event_queue; }

	auto next_frame() -> vk::CommandBuffer;
	void present(kvf::RenderTarget const& render_target);

  private:
	[[nodiscard]] static auto self(GLFWwindow* window) -> RenderWindow&;

	[[nodiscard]] auto create_window(glm::ivec2 size, klib::CString title) -> kvf::UniqueWindow;
	static void set_glfw_callbacks(GLFWwindow* window);

	kvf::UniqueWindow m_window;
	kvf::RenderDevice m_render_device;

	std::vector<Event> m_event_queue{};
};
} // namespace le
