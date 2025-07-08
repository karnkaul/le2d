#pragma once
#include <glm/vec2.hpp>
#include <klib/c_string.hpp>
#include <klib/enum_ops.hpp>
#include <kvf/render_device.hpp>
#include <kvf/window.hpp>
#include <le2d/event.hpp>
#include <variant>

namespace le {
enum class WindowFlag : std::uint8_t;
}

template <>
inline constexpr auto klib::enable_enum_ops_v<le::WindowFlag> = true;

namespace le {
/// \brief Window creation flags.
enum class WindowFlag : std::uint8_t {
	None = 0,
	/// \brief Window is decorated (has title bar, close button, etc).
	Decorated = 1 << 0,
	/// \brief Window is resizable.
	Resizeable = 1 << 1,
	/// \brief Window is visible on creation.
	Visible = 1 << 2,
	/// \brief Window is maximized on creation.
	Maximized = 1 << 3,
	/// \brief Window is given input focus when shown.
	FocusOnShow = 1 << 4,
	/// \brief Content area is resized based on content scale changes.
	ScaleToMonitor = 1 << 5,
	/// \brief Framebuffer is resized based on content scale changes.
	ScaleFramebuffer = 1 << 6,
};

/// \brief Default Window creation flags.
inline constexpr auto default_window_flags_v = WindowFlag::Decorated | WindowFlag::Resizeable | WindowFlag::Visible;

/// \brief Windowed RenderWindow parameters.
struct WindowInfo {
	glm::ivec2 size{600};
	klib::CString title;
	WindowFlag flags{default_window_flags_v};
};

/// \brief Fullscreen RenderWindow parameters.
struct FullscreenInfo {
	klib::CString title;
};

/// \brief RenderWindow creation parameters.
using WindowCreateInfo = std::variant<WindowInfo, FullscreenInfo>;

/// \brief Opaque interface to the render window.
/// Owns the Window and the RenderDevice.
/// Provides a view into the Window's Event Queue.
class IRenderWindow : public klib::Polymorphic {
  public:
	/// \returns Window size as reported by GLFW.
	[[nodiscard]] auto window_size() const -> glm::ivec2;
	/// \returns Framebuffer size as reported by GLFW.
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;
	/// \returns Ratio of framebuffer to window sizes.
	[[nodiscard]] auto display_ratio() const -> glm::vec2;

	/// \brief Check if Window is (and should remain) open.
	/// \returns true unless the close flag has been set.
	[[nodiscard]] auto is_open() const -> bool;
	/// \brief Set the close flag.
	/// Note: the window will remain visible until this object is destroyed by owning code.
	void set_closing();
	/// \brief Clear the close flag (if set).
	void cancel_close();

	/// \returns Events that occurred since the last frame.
	[[nodiscard]] virtual auto event_queue() const -> std::span<Event const> = 0;

	/// \brief Begin the next frame.
	/// Resets render resources and polls events.
	/// \returns Current virtual frame's Command Buffer.
	virtual auto next_frame() -> vk::CommandBuffer = 0;
	/// \brief Submit recorded commands and present a Render Target to the screen.
	virtual void present(kvf::RenderTarget const& render_target) = 0;

	[[nodiscard]] auto get_title() const -> klib::CString;
	void set_title(klib::CString title) const;

	[[nodiscard]] auto get_refresh_rate() const -> std::int32_t;

	[[nodiscard]] auto is_fullscreen() const -> bool;
	auto set_fullscreen(GLFWmonitor* target = nullptr) -> bool;
	void set_windowed(glm::ivec2 size = {1280, 720});

	[[nodiscard]] virtual auto get_render_device() const -> kvf::RenderDevice const& = 0;
	[[nodiscard]] auto get_render_device() -> kvf::RenderDevice&;
	[[nodiscard]] virtual auto get_window() const -> GLFWwindow* = 0;
};
} // namespace le
