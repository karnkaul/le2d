#pragma once
#include <detail/pipeline_pool.hpp>
#include <detail/resource/texture.hpp>
#include <kvf/device_waiter.hpp>
#include <kvf/render_device.hpp>
#include <le2d/resource/texture.hpp>

namespace le::detail {
class ResourcePool {
  public:
	explicit ResourcePool(gsl::not_null<kvf::RenderDevice*> render_device, std::unique_ptr<IShader> default_shader)
		: m_pipelines(render_device), m_default_shader(std::move(default_shader)), m_white_texture(render_device), m_waiter(render_device->get_device()) {}

	[[nodiscard]] auto allocate_pipeline(PipelineFixedState const& state, IShader const& shader) -> vk::Pipeline { return m_pipelines.allocate(state, shader); }

	[[nodiscard]] auto get_pipeline_layout() const -> vk::PipelineLayout { return m_pipelines.get_layout(); }
	[[nodiscard]] auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const> { return m_pipelines.get_set_layouts(); }
	[[nodiscard]] auto get_white_texture() const -> ITexture const& { return m_white_texture; }
	[[nodiscard]] auto get_default_shader() const -> IShader const& { return *m_default_shader; }

	[[nodiscard]] auto descriptor_image(ITextureBase const* texture) const -> vk::DescriptorImageInfo {
		return texture && texture->is_ready() ? texture->descriptor_info() : get_white_texture().descriptor_info();
	}

	std::vector<std::byte> scratch_buffer{};

  private:
	PipelinePool m_pipelines;
	std::unique_ptr<IShader> m_default_shader{};

	Texture m_white_texture;

	kvf::DeviceWaiter m_waiter;
};
} // namespace le::detail
