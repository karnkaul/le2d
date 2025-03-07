#pragma once
#include <capo/capo.hpp>
#include <le2d/asset/load_task.hpp>
#include <le2d/audio.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/font.hpp>
#include <le2d/frame_stats.hpp>
#include <le2d/render_pass.hpp>
#include <le2d/render_window.hpp>
#include <le2d/resource_pool.hpp>
#include <le2d/shader.hpp>
#include <le2d/vsync.hpp>

namespace le {
struct ContextCreateInfo {
	WindowCreateInfo window;
	struct {
		std::string_view vertex;
		std::string_view fragment;
	} default_shader_uri;

	kvf::RenderDeviceCreateInfo render_device{};
	vk::SampleCountFlagBits framebuffer_samples{vk::SampleCountFlagBits::e2};
	int sfx_buffers{16};
};

class Context : klib::Pinned {
  public:
	using CreateInfo = ContextCreateInfo;

	static constexpr auto min_render_scale_v{0.2f};
	static constexpr auto max_render_scale_v{8.0f};

	~Context() = default;

	Context(Context const&) = delete;
	Context(Context&&) = delete;
	auto operator=(Context const&) = delete;
	auto operator=(Context&&) = delete;

	explicit Context(gsl::not_null<IDataLoader const*> data_loader, CreateInfo const& create_info = {});

	[[nodiscard]] auto get_render_window() const -> RenderWindow const& { return m_window; }
	[[nodiscard]] auto get_data_loader() const -> IDataLoader const& { return *m_data_loader; }
	[[nodiscard]] auto get_resource_pool() const -> IResourcePool const& { return *m_resource_pool; }
	[[nodiscard]] auto get_audio() const -> IAudio& { return *m_audio; }
	[[nodiscard]] auto get_default_shader() const -> Shader const& { return m_resource_pool->get_default_shader(); }

	[[nodiscard]] auto swapchain_size() const -> glm::ivec2 { return m_window.framebuffer_size(); }
	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2;
	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_window.event_queue(); }

	[[nodiscard]] auto is_running() const -> bool { return m_window.is_open(); }
	void shutdown() { m_window.set_closing(); }

	[[nodiscard]] auto get_render_scale() const -> float { return m_render_scale; }
	auto set_render_scale(float scale) -> bool;

	[[nodiscard]] auto get_supported_vsync() const -> std::span<Vsync const> { return m_supported_vsync; }
	[[nodiscard]] auto get_vsync() const -> Vsync;
	auto set_vsync(Vsync vsync) -> bool;

	auto next_frame() -> vk::CommandBuffer;
	[[nodiscard]] auto begin_render(kvf::Color clear = kvf::black_v) -> Renderer;
	void present();

	[[nodiscard]] auto get_frame_stats() const -> FrameStats const& { return m_frame_stats; }

	[[nodiscard]] auto create_device_block() const -> kvf::DeviceBlock { return m_window.get_render_device().get_device(); }
	[[nodiscard]] auto create_shader(Uri const& vertex, Uri const& fragment) const -> Shader;
	[[nodiscard]] auto create_render_pass(vk::SampleCountFlagBits samples) const -> RenderPass;
	[[nodiscard]] auto create_texture(kvf::Bitmap bitmap = {}) const -> Texture;
	[[nodiscard]] auto create_font(std::vector<std::byte> font_bytes = {}) const -> Font;
	[[nodiscard]] auto create_asset_load_task(gsl::not_null<klib::task::Queue*> task_queue) const -> std::unique_ptr<asset::LoadTask>;

  private:
	struct Fps {
		std::int32_t counter{};
		std::int32_t value{};
		kvf::Seconds elapsed{};
	};

	IDataLoader const* m_data_loader;

	RenderWindow m_window;
	RenderPass m_pass;
	std::vector<Vsync> m_supported_vsync{};

	std::unique_ptr<IResourcePool> m_resource_pool{};
	std::unique_ptr<IAudio> m_audio{};

	float m_render_scale{1.0f};

	vk::CommandBuffer m_cmd{};

	kvf::Clock::time_point m_frame_start{};
	Fps m_fps{};
	FrameStats m_frame_stats{};
};
} // namespace le
