#pragma once
#include <glm/vec2.hpp>
#include <klib/c_string.hpp>
#include <kvf/device_block.hpp>
#include <kvf/render_device.hpp>
#include <kvf/window.hpp>
#include <le2d/event.hpp>
#include <variant>

namespace le {
struct WindowInfo {
	glm::ivec2 size{600};
	klib::CString title;
	bool decorated{true};
};

struct FullscreenInfo {
	klib::CString title;
	GLFWmonitor* target{nullptr};
};

using WindowCreateInfo = std::variant<WindowInfo, FullscreenInfo>;

class RenderWindow {
  public:
	explicit RenderWindow(WindowCreateInfo const& wci, kvf::RenderDeviceCreateInfo const& rdci = {});

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice const& { return m_render_device; }
	[[nodiscard]] auto get_render_device() -> kvf::RenderDevice& { return m_render_device; }
	[[nodiscard]] auto get_window() const -> GLFWwindow* { return m_window.get(); }

	[[nodiscard]] auto window_size() const -> glm::ivec2;
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;

	[[nodiscard]] auto is_open() const -> bool;
	void set_closing();
	void cancel_close();

	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_event_queue; }

	[[nodiscard]] auto display_ratio() const -> glm::vec2;

	auto next_frame() -> vk::CommandBuffer;
	void present(kvf::RenderTarget const& render_target);

	[[nodiscard]] auto get_title() const -> klib::CString;
	void set_title(klib::CString title) const;

	[[nodiscard]] auto get_refresh_rate() const -> std::int32_t;

	[[nodiscard]] auto is_fullscreen() const -> bool;
	auto set_fullscreen(GLFWmonitor* target = nullptr) -> bool;
	void set_windowed(glm::ivec2 size = {1280, 720});

  private:
	[[nodiscard]] static auto self(GLFWwindow* window) -> RenderWindow&;

	[[nodiscard]] auto create_window(WindowCreateInfo const& wci) -> kvf::UniqueWindow;

	static void set_glfw_callbacks(GLFWwindow* window);

	void on_cursor_pos(window::vec2 pos);
	void on_drop(int count, char const** paths);

	kvf::UniqueWindow m_window;
	kvf::RenderDevice m_render_device;

	std::vector<Event> m_event_queue{};
	std::vector<std::string> m_drops{};
};
} // namespace le
