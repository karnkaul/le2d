#pragma once
#include <GLFW/glfw3.h>
#include <klib/enum_ops.hpp>
#include <kvf/render_device.hpp>
#include <kvf/window.hpp>
#include <le2d/asset/asset_loader.hpp>
#include <le2d/audio_mixer.hpp>
#include <le2d/build_version.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/event.hpp>
#include <le2d/frame_stats.hpp>
#include <le2d/render_pass.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <le2d/vsync.hpp>
#include <variant>

namespace le {
enum class PlatformFlag : std::uint8_t;
enum class WindowFlag : std::uint8_t;
} // namespace le

template <>
inline constexpr auto klib::enable_enum_ops_v<le::WindowFlag> = true;
template <>
inline constexpr auto klib::enable_enum_ops_v<le::PlatformFlag> = true;

namespace le {
/// \brief Platform flags (GLFW init hints).
enum class PlatformFlag : std::uint8_t {
	None = 0,
	/// \brief Force X11 backend instead of Wayland (only relevant on Linux).
	ForceX11,
	/// \brief Disable libdecor (only relevant on Wayland).
	NoLibdecor,
};

/// \brief Window creation flags (GLFW window hints).
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

/// \brief Windowed window parameters.
struct WindowInfo {
	glm::ivec2 size{600};
	klib::CString title;
	WindowFlag flags{default_window_flags_v};
};

/// \brief Fullscreen window parameters.
struct FullscreenInfo {
	klib::CString title;
};

/// \brief Window creation parameters.
using WindowCreateInfo = std::variant<WindowInfo, FullscreenInfo>;

/// \brief Context creation parameters.
struct ContextCreateInfo {
	/// \brief Platform flags.
	PlatformFlag platform_flags{PlatformFlag::None};
	/// \brief Window creation parameters.
	WindowCreateInfo window{WindowInfo{}};
	/// \brief Render Device creation parameters.
	kvf::RenderDeviceCreateInfo render_device{};
	/// \brief Multi sampled anti-aliasing.
	vk::SampleCountFlagBits framebuffer_samples{vk::SampleCountFlagBits::e2};
	/// \brief Number of SFX buffers (concurrently playable).
	int sfx_buffers{16};
};

/// \brief Central API for most of the engine / framework.
class Context : public klib::Polymorphic {
  public:
	/// \brief RAII wrapper over wait_idle().
	class Waiter;

	using SpirV = std::span<std::uint32_t const>;
	using CreateInfo = ContextCreateInfo;

	static constexpr auto min_render_scale_v{0.2f};
	static constexpr auto max_render_scale_v{8.0f};

	[[nodiscard]] static auto create(CreateInfo const& create_info = {}) -> std::unique_ptr<Context>;

	[[nodiscard]] virtual auto get_window() const -> GLFWwindow* = 0;
	[[nodiscard]] virtual auto get_render_device() const -> kvf::RenderDevice const& = 0;
	[[nodiscard]] virtual auto get_resource_factory() const -> IResourceFactory const& = 0;
	[[nodiscard]] virtual auto get_audio_mixer() const -> IAudioMixer& = 0;
	[[nodiscard]] virtual auto get_default_shader() const -> IShader const& = 0;
	[[nodiscard]] virtual auto get_renderer() const -> IRenderer const& = 0;

	/// \returns Window size as reported by GLFW.
	[[nodiscard]] auto window_size() const -> glm::ivec2;
	/// \returns Framebuffer size as reported by GLFW.
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;
	/// \returns Current Swapchain extent.
	[[nodiscard]] auto swapchain_extent() const -> vk::Extent2D;
	/// \returns Main Render Pass framebuffer size (scaled).
	[[nodiscard]] auto main_pass_size() const -> glm::ivec2;
	/// \returns Ratio of framebuffer to window sizes.
	[[nodiscard]] auto display_ratio() const -> glm::vec2;

	[[nodiscard]] auto get_title() const -> klib::CString;
	void set_title(klib::CString title);

	[[nodiscard]] auto get_refresh_rate() const -> std::int32_t;

	[[nodiscard]] auto is_fullscreen() const -> bool;
	/// \brief Show window and set fullscreen.
	/// \param target Target monitor (optional).
	/// \returns true if successful.
	auto set_fullscreen(GLFWmonitor* target = nullptr) -> bool;
	/// \brief Show window and set windowed with given size.
	/// \param size Window size. Must be positive.
	void set_windowed(glm::ivec2 size = {1280, 720});
	/// \brief Show/hide window.
	void set_visible(bool visible);

	/// \brief Check if Window is (and should remain) open.
	/// \returns true unless the close flag has been set.
	[[nodiscard]] auto is_running() const -> bool;
	/// \brief Set the Window close flag.
	/// Note: the window will remain visible until this object is destroyed by owning code.
	void set_window_close();
	/// \brief Reset the Window close flag.
	void cancel_window_close();

	/// \brief Wait for the graphics and audio devices to become idle.
	/// Does not account for user owned audio sources.
	void wait_idle();

	/// \returns Current render scale.
	[[nodiscard]] virtual auto get_render_scale() const -> float = 0;
	/// \param scale Desired render scale.
	/// \returns true if desired scale is within limits.
	virtual auto set_render_scale(float scale) -> bool = 0;

	/// \returns List of supported Vsync modes.
	[[nodiscard]] virtual auto get_supported_vsync() const -> std::span<Vsync const> = 0;
	/// \returns Current Vsync mode.
	[[nodiscard]] virtual auto get_vsync() const -> Vsync = 0;
	/// \param vsync Desired Vsync mode.
	/// \returns true if desired mode is supported.
	virtual auto set_vsync(Vsync vsync) -> bool = 0;

	/// \returns Current MSAA samples.
	[[nodiscard]] virtual auto get_samples() const -> vk::SampleCountFlagBits = 0;
	/// \returns Supported MSAA samples.
	[[nodiscard]] virtual auto get_supported_samples() const -> vk::SampleCountFlags = 0;
	/// \brief Set desired MSAA samples.
	/// RenderPass will be recreated on the next frame, not immediately.
	/// \returns true unless not supported.
	virtual auto set_samples(vk::SampleCountFlagBits samples) -> bool = 0;

	/// \brief Begin the next frame.
	/// Resets render resources and polls events.
	/// \returns Current virtual frame's Command Buffer.
	virtual auto next_frame() -> vk::CommandBuffer = 0;
	/// \returns Events that occurred since the last frame.
	[[nodiscard]] virtual auto event_queue() const -> std::span<Event const> = 0;
	/// \brief Begin rendering the primary RenderPass.
	/// \param clear Clear color.
	/// \returns Renderer instance.
	[[nodiscard]] virtual auto begin_render(kvf::Color clear = kvf::black_v) -> IRenderer& = 0;
	/// \brief Submit recorded commands and present RenderTarget of primary RenderPass.
	virtual void present() = 0;

	[[nodiscard]] virtual auto get_frame_stats() const -> FrameStats const& = 0;

	[[nodiscard]] auto create_waiter() -> Waiter;
	[[nodiscard]] virtual auto create_render_pass(vk::SampleCountFlagBits samples) const -> std::unique_ptr<IRenderPass> = 0;
	[[nodiscard]] virtual auto create_asset_loader(gsl::not_null<IDataLoader const*> data_loader) const -> AssetLoader = 0;
};

/// \brief Calls Context::wait_idle() in its destructor.
class Context::Waiter {
  public:
	Waiter() = default;

	explicit(false) Waiter(gsl::not_null<Context*> context) : m_context(context) {}

	[[nodiscard]] auto is_active() const -> bool { return m_context != nullptr; }

	[[nodiscard]] auto get_context() const -> Context* { return m_context.get(); }

  private:
	struct Deleter {
		void operator()(Context* ptr) const {
			if (!ptr) { return; }
			ptr->wait_idle();
		}
	};

	std::unique_ptr<Context, Deleter> m_context{};
};
} // namespace le
