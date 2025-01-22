#pragma once
#include <le2d/data_loader.hpp>
#include <le2d/font.hpp>
#include <le2d/render_pass.hpp>
#include <le2d/render_window.hpp>
#include <le2d/resource_pool.hpp>
#include <le2d/shader.hpp>
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

struct ContextCreateInfo {
	WindowCreateInfo window;
	struct {
		Uri vertex;
		Uri fragment;
	} default_shader;

	vk::SampleCountFlagBits framebuffer_samples{vk::SampleCountFlagBits::e2};
};

class Context {
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

	[[nodiscard]] auto swapchain_size() const -> glm::ivec2 { return m_window.framebuffer_size(); }
	[[nodiscard]] auto event_queue() const -> std::span<Event const> { return m_window.event_queue(); }

	[[nodiscard]] auto is_running() const -> bool { return m_window.is_open(); }
	void shutdown() { m_window.set_closing(); }

	[[nodiscard]] auto get_render_scale() const -> float { return m_render_scale; }
	auto set_render_scale(float scale) -> bool;

	auto next_frame() -> vk::CommandBuffer;
	[[nodiscard]] auto begin_render() -> Renderer;
	void present();

	[[nodiscard]] auto create_device_block() const -> kvf::DeviceBlock { return m_window.get_render_device().get_device(); }
	[[nodiscard]] auto create_shader(Uri const& vertex, Uri const& fragment) const -> Shader;
	[[nodiscard]] auto create_render_pass(vk::SampleCountFlagBits samples) const -> RenderPass;
	[[nodiscard]] auto create_texture(kvf::Bitmap const& bitmap = {}) const -> Texture;
	[[nodiscard]] auto create_font(std::vector<std::byte> font_bytes = {}) const -> Font;

  private:
	IDataLoader const* m_data_loader;

	RenderWindow m_window;
	mutable ResourcePool m_resource_pool;
	RenderPass m_pass;

	float m_render_scale{1.0f};

	vk::CommandBuffer m_cmd{};
};
} // namespace le
