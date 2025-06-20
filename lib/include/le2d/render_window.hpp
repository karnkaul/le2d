#pragma once
#include <glm/vec2.hpp>
#include <klib/c_string.hpp>
#include <kvf/device_block.hpp>
#include <kvf/render_device.hpp>
#include <kvf/window.hpp>
#include <le2d/event.hpp>
#include <variant>

namespace le {
/// \brief Windowed RenderWindow parameters.
struct WindowInfo {
	glm::ivec2 size{600};
	klib::CString title;
	bool decorated{true};
};

/// \brief Fullscreen RenderWindow parameters.
struct FullscreenInfo {
	klib::CString title;
	GLFWmonitor* target{nullptr};
};

/// \brief RenderWindow creation parameters.
using WindowCreateInfo = std::variant<WindowInfo, FullscreenInfo>;

/// \brief Owns the Window and the RenderDevice.
/// Provides a view into the Window's Event Queue.
/// A separately owned RenderPass is required to render to the Window.
class RenderWindow {
  public:
	/// \param wci Window creation parameters (windowed or fullscreen).
	/// \param rdci Render Device creation parameters.
	explicit RenderWindow(WindowCreateInfo const& wci, kvf::RenderDeviceCreateInfo const& rdci = {});

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice const& { return m_render_device; }
	[[nodiscard]] auto get_render_device() -> kvf::RenderDevice& { return m_render_device; }
	[[nodiscard]] auto get_window() const -> GLFWwindow* { return m_window.get(); }

	/// \returns Window size as reported by GLFW.
	[[nodiscard]] auto window_size() const -> glm::ivec2;
	/// \returns Framebuffer size as reported by GLFW.
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;

	/// \brief Check if Window is (and should remain) open.
	/// \returns true unless the close flag has been set.
	[[nodiscard]] auto is_open() const -> bool;
	/// \brief Set the close flag.
	/// Note: the window will remain visible until this object is destroyed by owning code.
	void set_closing();
	/// \brief Clear the close flag (if set).
	void cancel_close();

	/// \returns Events that occurred since the last frame.
	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_event_queue; }

	/// \returns Ratio of framebuffer to window sizes.
	[[nodiscard]] auto display_ratio() const -> glm::vec2;

	/// \brief Begin the next frame.
	/// Resets render resources and polls events.
	/// \returns Current virtual frame's Command Buffer.
	auto next_frame() -> vk::CommandBuffer;
	/// \brief Submit recorded commands and present a Render Target to the screen.
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
