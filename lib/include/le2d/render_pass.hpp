#pragma once
#include <glm/vec2.hpp>
#include <kvf/device_block.hpp>
#include <kvf/render_pass.hpp>
#include <le2d/renderer.hpp>
#include <le2d/resource_pool.hpp>

namespace le {
/// \brief 2D render pass, owns a multi-sampled color Render Target.
class RenderPass {
  public:
	static constexpr auto min_size_v{32};
	static constexpr auto max_size_v{4 * 4096};

	/// \param render_device Pointer to persistent RenderDevice.
	/// \param samples MSAA samples for color target.
	explicit RenderPass(gsl::not_null<kvf::RenderDevice*> render_device, vk::SampleCountFlagBits samples);

	[[nodiscard]] auto get_render_target() const -> kvf::RenderTarget const& { return m_render_pass.render_target(); }

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice& { return *m_render_device; }
	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits { return m_render_pass.get_samples(); }

	/// \brief Set clear color for next pass.
	/// \param color Clear color.
	void set_clear_color(kvf::Color color);

	/// \brief Begin rendering.
	/// \param resource_pool Resource Pool.
	/// \param command_buffer Recording Command Buffer.
	/// \param size Desired framebuffer size. Must be positive.
	/// \returns Renderer instance.
	auto begin_render(IResourcePool& resource_pool, vk::CommandBuffer command_buffer, glm::ivec2 size) -> Renderer;

  private:
	kvf::RenderDevice* m_render_device;
	kvf::RenderPass m_render_pass;

	kvf::DeviceBlock m_blocker;
};
} // namespace le
