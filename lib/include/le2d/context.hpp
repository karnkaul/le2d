#pragma once
#include <le2d/asset/asset_loader.hpp>
#include <le2d/audio_mixer.hpp>
#include <le2d/build_version.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/frame_stats.hpp>
#include <le2d/gamepad.hpp>
#include <le2d/render_pass.hpp>
#include <le2d/render_window.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <le2d/resource/resource_pool.hpp>
#include <le2d/vsync.hpp>

namespace le {
/// \brief Context creation parameters.
struct ContextCreateInfo {
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
class Context : public klib::Pinned {
  public:
	/// \brief RAII wrapper over wait_idle().
	class Waiter;

	using SpirV = std::span<std::uint32_t const>;
	using CreateInfo = ContextCreateInfo;

	static constexpr auto min_render_scale_v{0.2f};
	static constexpr auto max_render_scale_v{8.0f};

	/// \param create_info Creation parameters.
	explicit Context(CreateInfo const& create_info = {});

	[[nodiscard]] auto get_render_window() const -> IRenderWindow const& { return *m_window; }
	[[nodiscard]] auto get_resource_factory() const -> IResourceFactory const& { return *m_resource_factory; }
	[[nodiscard]] auto get_resource_pool() const -> IResourcePool& { return *m_resource_pool; }
	[[nodiscard]] auto get_audio_mixer() const -> IAudioMixer& { return *m_audio_mixer; }
	[[nodiscard]] auto get_default_shader() const -> IShader const& { return m_resource_pool->get_default_shader(); }

	/// \brief Get the updated state of the last used Gamepad (if any).
	/// \returns If unset or disconnected, le::Gamepad::get_active(), else updated Gamepad state.
	[[nodiscard]] auto get_latest_gamepad() -> Gamepad const&;

	/// \returns Current size of swapchain images.
	[[nodiscard]] auto swapchain_size() const -> glm::ivec2 { return m_window->framebuffer_size(); }
	/// \returns Scaled render framebuffer size.
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;
	/// \returns Events that occurred since the last frame.
	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_window->event_queue(); }

	/// \brief Check if Window is (and should remain) open.
	/// \returns true unless the close flag has been set.
	[[nodiscard]] auto is_running() const -> bool { return m_window->is_open(); }
	/// \brief Set the Window close flag.
	/// Note: the window will remain visible until this object is destroyed by owning code.
	void shutdown() { m_window->set_closing(); }
	/// \brief Reset the Window close flag.
	void cancel_window_close() { m_window->cancel_close(); }

	/// \returns Current render scale.
	[[nodiscard]] auto get_render_scale() const -> float { return m_render_scale; }
	/// \param scale Desired render scale.
	/// \returns true if desired scale is within limits.
	auto set_render_scale(float scale) -> bool;

	/// \returns List of supported Vsync modes.
	[[nodiscard]] auto get_supported_vsync() const -> std::span<Vsync const> { return m_supported_vsync; }
	/// \returns Current Vsync mode.
	[[nodiscard]] auto get_vsync() const -> Vsync;
	/// \param vsync Desired Vsync mode.
	/// \returns true if desired mode is supported.
	auto set_vsync(Vsync vsync) -> bool;

	auto set_fullscreen(GLFWmonitor* target = nullptr) -> bool { return m_window->set_fullscreen(target); }
	void set_windowed(glm::ivec2 const size = {1280, 720}) { m_window->set_windowed(size); }
	void set_visible(bool const visible) { m_window->set_visible(visible); }

	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits { return m_pass.get_samples(); }
	[[nodiscard]] auto get_supported_samples() const -> vk::SampleCountFlags;
	auto set_samples(vk::SampleCountFlagBits samples) -> bool;

	/// \brief Begin the next frame.
	/// Resets render resources and polls events.
	/// \returns Current virtual frame's Command Buffer.
	auto next_frame() -> vk::CommandBuffer;
	/// \brief Begin rendering the primary RenderPass.
	/// \param clear Clear color.
	/// \returns Renderer instance.
	[[nodiscard]] auto begin_render(kvf::Color clear = kvf::black_v) -> Renderer;
	/// \brief Submit recorded commands and present RenderTarget of primary RenderPass.
	void present();

	/// \brief Wait for the graphics and audio devices to become idle.
	/// Does not account for user owned audio sources.
	void wait_idle();

	[[nodiscard]] auto get_frame_stats() const -> FrameStats const& { return m_frame_stats; }

	[[nodiscard]] auto create_waiter() -> Waiter;
	[[nodiscard]] auto create_render_pass(vk::SampleCountFlagBits samples) const -> RenderPass;
	[[nodiscard]] auto create_asset_loader(gsl::not_null<IDataLoader const*> data_loader) const -> AssetLoader;

  private:
	struct OnDestroy {
		void operator()(Context* ptr) const noexcept;
	};

	struct Fps {
		std::int32_t counter{};
		std::int32_t value{};
		kvf::Seconds elapsed{};
	};

	struct Requests {
		[[nodiscard]] auto is_empty() const -> bool { return !set_samples; }

		std::optional<vk::SampleCountFlagBits> set_samples{};
	};

	void process_requests();

	void update_stats(kvf::Clock::time_point present_start);

	template <typename... Ts>
	void add_loaders(AssetLoader& out, IDataLoader const& data_loader) const;

	std::unique_ptr<IRenderWindow> m_window{};
	RenderPass m_pass;
	std::vector<Vsync> m_supported_vsync{};

	std::unique_ptr<IResourceFactory> m_resource_factory{};
	std::unique_ptr<IResourcePool> m_resource_pool{};
	std::unique_ptr<IAudioMixer> m_audio_mixer{};

	Requests m_requests{};

	Gamepad m_latest_gamepad{};

	float m_render_scale{1.0f};

	vk::CommandBuffer m_cmd{};

	kvf::Clock::time_point m_frame_start{};
	kvf::Clock::time_point m_runtime_start{kvf::Clock::now()};
	Fps m_fps{};
	FrameStats m_frame_stats{};

	std::unique_ptr<Context, OnDestroy> m_on_destroy{};
};

/// \brief Calls Context::wait_idle() in its destructor.
class Context::Waiter {
  public:
	Waiter() = default;

	Waiter(std::nullptr_t) = delete;

	explicit(false) Waiter(Context* context) : m_context(context) {}

	[[nodiscard]] auto is_active() const -> bool { return m_context != nullptr; }

	[[nodiscard]] auto get_context() const -> Context* { return m_context.get(); }

  private:
	struct Deleter {
		void operator()(Context* ptr) const;
	};

	std::unique_ptr<Context, Deleter> m_context{};
};
} // namespace le
