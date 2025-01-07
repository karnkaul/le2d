#pragma once
#include <glm/vec2.hpp>
#include <kvf/device_block.hpp>
#include <kvf/render_pass.hpp>
#include <le2d/renderer.hpp>

namespace le {
class ResourcePool;
class Renderer;

class RenderPass {
  public:
	static constexpr auto min_size_v{32};
	static constexpr auto max_size_v{4 * 4096};

	explicit RenderPass(gsl::not_null<kvf::RenderDevice*> render_device, gsl::not_null<ResourcePool*> resource_pool, vk::SampleCountFlagBits samples);

	[[nodiscard]] auto get_render_target() const -> kvf::RenderTarget const& { return m_render_pass.render_target(); }

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice& { return *m_render_device; }
	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits { return m_render_pass.get_samples(); }

	void set_clear_color(kvf::Color color);

	auto begin_render(vk::CommandBuffer command_buffer, glm::ivec2 size) -> Renderer;

  private:
	kvf::RenderDevice* m_render_device;
	ResourcePool* m_resource_pool;
	kvf::RenderPass m_render_pass;

	kvf::DeviceBlock m_blocker;

	friend class Renderer;
};
} // namespace le
