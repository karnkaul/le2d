#pragma once
#include <glm/vec2.hpp>
#include <kvf/device_waiter.hpp>
#include <kvf/render_pass.hpp>
#include <le2d/renderer.hpp>
#include <le2d/resource/resource_pool.hpp>
#include <memory>

namespace le {
/// \brief 2D render pass, owns a multi-sampled color RenderTarget.
class RenderPass {
  public:
	static constexpr auto min_size_v{32};
	static constexpr auto max_size_v{4 * 4096};

	/// \param render_device Pointer to persistent RenderDevice.
	/// \param samples MSAA samples for color target.
	explicit RenderPass(gsl::not_null<kvf::RenderDevice*> render_device, vk::SampleCountFlagBits samples);

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice& { return *m_render_device; }

	/// \returns true unless moved.
	[[nodiscard]] auto is_ready() const -> bool { return m_render_pass != nullptr; }

	/// \returns Reference to latest RenderTarget.
	/// Can only be called when is_ready() returns true.
	[[nodiscard]] auto get_render_target() const -> kvf::RenderTarget const&;

	/// \returns Multi-sampling count.
	/// Can only be called when is_ready() returns true.
	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits;

	/// \returns RenderTarget as a texture. Must not outlive RenderPass.
	/// Can only be called when is_ready() returns true.
	[[nodiscard]] auto render_texture() const -> RenderTexture;

	/// \brief Set clear color for next pass.
	/// \param color Clear color.
	/// Can only be called when is_ready() returns true.
	void set_clear_color(kvf::Color color);

	/// \brief Begin rendering.
	/// \param resource_pool Resource Pool.
	/// \param command_buffer Recording Command Buffer.
	/// \param size Desired framebuffer size. Must be positive.
	/// \returns Renderer instance.
	/// Can only be called when is_ready() returns true.
	auto begin_render(IResourcePool& resource_pool, vk::CommandBuffer command_buffer, glm::ivec2 size) -> Renderer;

  private:
	gsl::not_null<kvf::RenderDevice*> m_render_device;
	std::unique_ptr<kvf::RenderPass> m_render_pass{};

	kvf::DeviceWaiter m_waiter;
};
} // namespace le
