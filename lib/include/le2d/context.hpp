#pragma once
#include <le2d/asset/asset_loader.hpp>
#include <le2d/audio_mixer.hpp>
#include <le2d/build_version.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/frame_stats.hpp>
#include <le2d/render_pass.hpp>
#include <le2d/render_window.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <le2d/resource/resource_pool.hpp>
#include <le2d/vsync.hpp>

namespace le {
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
/// Encapsulates RenderWindow, primary RenderPass, Audio Engine.
class IContext : public klib::Polymorphic {
  public:
	/// \brief RAII wrapper over wait_idle().
	class Waiter;

	using SpirV = std::span<std::uint32_t const>;
	using CreateInfo = ContextCreateInfo;

	static constexpr auto min_render_scale_v{0.2f};
	static constexpr auto max_render_scale_v{8.0f};

	[[nodiscard]] static auto create(CreateInfo const& create_info = {}) -> std::unique_ptr<IContext>;

	[[nodiscard]] virtual auto get_render_window() const -> IRenderWindow const& = 0;
	[[nodiscard]] virtual auto get_resource_factory() const -> IResourceFactory const& = 0;
	[[nodiscard]] virtual auto get_audio_mixer() const -> IAudioMixer& = 0;
	[[nodiscard]] virtual auto get_default_shader() const -> IShader const& = 0;

	/// \returns Current size of swapchain images.
	[[nodiscard]] virtual auto swapchain_size() const -> glm::ivec2 = 0;
	/// \returns Scaled render framebuffer size.
	[[nodiscard]] virtual auto framebuffer_size() const -> glm::ivec2 = 0;
	/// \returns Events that occurred since the last frame.
	[[nodiscard]] virtual auto event_queue() const -> std::span<Event const> = 0;

	/// \brief Check if Window is (and should remain) open.
	/// \returns true unless the close flag has been set.
	[[nodiscard]] virtual auto is_running() const -> bool = 0;
	/// \brief Set the Window close flag.
	/// Note: the window will remain visible until this object is destroyed by owning code.
	virtual void shutdown() = 0;
	/// \brief Reset the Window close flag.
	virtual void cancel_window_close() = 0;

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

	/// \brief Show window and set fullscreen.
	/// \param target Target monitor (optional).
	/// \returns true if successful.
	virtual auto set_fullscreen(GLFWmonitor* target = nullptr) -> bool = 0;
	/// \brief Show window and set windowed with given size.
	/// \param size Window size. Must be positive.
	virtual void set_windowed(glm::ivec2 size = {1280, 720}) = 0;
	/// \brief Show/hide window.
	virtual void set_visible(bool visible) = 0;

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
	/// \brief Begin rendering the primary RenderPass.
	/// \param clear Clear color.
	/// \returns Renderer instance.
	[[nodiscard]] virtual auto begin_render(kvf::Color clear = kvf::black_v) -> IRenderer& = 0;
	/// \brief Submit recorded commands and present RenderTarget of primary RenderPass.
	virtual void present() = 0;

	/// \brief Wait for the graphics and audio devices to become idle.
	/// Does not account for user owned audio sources.
	virtual void wait_idle() = 0;

	[[nodiscard]] virtual auto get_frame_stats() const -> FrameStats const& = 0;

	[[nodiscard]] auto create_waiter() -> Waiter;
	[[nodiscard]] virtual auto create_render_pass(vk::SampleCountFlagBits samples) const -> std::unique_ptr<IRenderPass> = 0;
	[[nodiscard]] virtual auto create_asset_loader(gsl::not_null<IDataLoader const*> data_loader) const -> AssetLoader = 0;
};

/// \brief Calls Context::wait_idle() in its destructor.
class IContext::Waiter {
  public:
	Waiter() = default;

	explicit(false) Waiter(gsl::not_null<IContext*> context) : m_context(context) {}

	[[nodiscard]] auto is_active() const -> bool { return m_context != nullptr; }

	[[nodiscard]] auto get_context() const -> IContext* { return m_context.get(); }

  private:
	struct Deleter {
		void operator()(IContext* ptr) const {
			if (!ptr) { return; }
			ptr->wait_idle();
		}
	};

	std::unique_ptr<IContext, Deleter> m_context{};
};
} // namespace le
