#pragma once
#include "detail/resource/resource_factory.hpp"
#include "detail/resource/texture.hpp"
#include "detail/shader_layout.hpp"
#include "kvf/device_waiter.hpp"
#include "le2d/render_instance.hpp"
#include "le2d/resource/shader.hpp"

namespace le::detail {
class ResourcePool {
  public:
	explicit ResourcePool(gsl::not_null<ResourceFactory const*> resource_factory);

	[[nodiscard]] auto get_white_texture() const -> ITexture const& { return m_white_texture; }
	[[nodiscard]] auto get_default_shader() const -> IShader const& { return *m_default_shader; }

	[[nodiscard]] auto descriptor_image(ITextureBase const* texture) const -> vk::DescriptorImageInfo;

	gsl::not_null<ISamplerFactory*> sampler_factory;
	gsl::not_null<ShaderLayout const*> shader_layout;

	std::vector<RenderInstance::Std430> render_instance_buffer{};

  private:
	std::unique_ptr<IShader> m_default_shader{};

	Texture m_white_texture;

	kvf::DeviceWaiter m_waiter;
};
} // namespace le::detail
