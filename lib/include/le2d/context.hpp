#pragma once
#include <le2d/audio.hpp>
#include <le2d/font.hpp>
#include <le2d/frame_stats.hpp>
#include <le2d/gamepad.hpp>
#include <le2d/render_pass.hpp>
#include <le2d/render_window.hpp>
#include <le2d/resource_pool.hpp>
#include <le2d/shader_program.hpp>
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
	using SpirV = std::span<std::uint32_t const>;
	using CreateInfo = ContextCreateInfo;

	static constexpr auto min_render_scale_v{0.2f};
	static constexpr auto max_render_scale_v{8.0f};

	/// \param create_info Creation parameters.
	explicit Context(CreateInfo const& create_info = {});

	[[nodiscard]] auto get_render_window() const -> RenderWindow const& { return m_window; }
	[[nodiscard]] auto get_resource_pool() const -> IResourcePool const& { return *m_resource_pool; }
	[[nodiscard]] auto get_audio() const -> IAudio& { return *m_audio; }
	[[nodiscard]] auto get_default_shader() const -> ShaderProgram const& { return m_resource_pool->get_default_shader(); }

	/// \brief Get the updated state of the last used Gamepad (if any).
	/// \returns If unset or disconnected, le::Gamepad::get_active(), else updated Gamepad state.
	[[nodiscard]] auto get_latest_gamepad() -> Gamepad const&;

	/// \returns Current size of swapchain images.
	[[nodiscard]] auto swapchain_size() const -> glm::ivec2 { return m_window.framebuffer_size(); }
	/// \returns Scaled render framebuffer size.
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;
	/// \returns Events that occurred since the last frame.
	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_window.event_queue(); }

	/// \brief Check if Window is (and should remain) open.
	/// \returns true unless the close flag has been set.
	[[nodiscard]] auto is_running() const -> bool { return m_window.is_open(); }
	/// \brief Set the Window close flag.
	/// Note: the window will remain visible until this object is destroyed by owning code.
	void shutdown() { m_window.set_closing(); }
	/// \brief Reset the Window close flag.
	void cancel_window_close() { m_window.cancel_close(); }

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

	auto set_fullscreen(GLFWmonitor* target = nullptr) -> bool { return m_window.set_fullscreen(target); }
	void set_windowed(glm::ivec2 const size = {1280, 720}) { m_window.set_windowed(size); }

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

	[[nodiscard]] auto get_frame_stats() const -> FrameStats const& { return m_frame_stats; }

	[[nodiscard]] auto create_device_block() const -> kvf::DeviceBlock { return m_window.get_render_device().get_device(); }
	[[nodiscard]] auto create_shader(SpirV vertex, SpirV fragment) const -> ShaderProgram;
	[[nodiscard]] auto create_render_pass(vk::SampleCountFlagBits samples) const -> RenderPass;
	[[nodiscard]] auto create_texture(kvf::Bitmap const& bitmap = {}) const -> Texture;
	[[nodiscard]] auto create_tilesheet(kvf::Bitmap const& bitmap = {}) const -> TileSheet;
	[[nodiscard]] auto create_font(std::vector<std::byte> font_bytes = {}) const -> Font;

  private:
	struct OnDestroy {
		void operator()(int i) const noexcept;
	};

	struct Fps {
		std::int32_t counter{};
		std::int32_t value{};
		kvf::Seconds elapsed{};
	};

	void update_stats(kvf::Clock::time_point present_start);

	RenderWindow m_window;
	RenderPass m_pass;
	std::vector<Vsync> m_supported_vsync{};

	std::unique_ptr<IResourcePool> m_resource_pool{};
	std::unique_ptr<IAudio> m_audio{};

	Gamepad m_latest_gamepad{};

	float m_render_scale{1.0f};

	vk::CommandBuffer m_cmd{};

	kvf::Clock::time_point m_frame_start{};
	kvf::Clock::time_point m_runtime_start{kvf::Clock::now()};
	Fps m_fps{};
	FrameStats m_frame_stats{};

	klib::Unique<int, OnDestroy> m_on_destroy{};

	kvf::DeviceBlock m_blocker;
};
} // namespace le
