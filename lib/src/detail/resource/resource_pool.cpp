#include "detail/resource/resource_pool.hpp"
#include "detail/resource/shader.hpp"
#include "le2d/error.hpp"
#include "spirv.hpp"

namespace le::detail {
namespace {
[[nodiscard]] auto create_default_shader(gsl::not_null<kvf::IRenderDevice*> render_device, ShaderLayout const& layout) -> std::unique_ptr<IShader> {
	auto const vert_spirv = spirv::vert();
	auto const frag_spirv = spirv::frag();
	auto ret = std::make_unique<Shader>(render_device, layout.get_set_layouts());
	if (!ret->load(vert_spirv, frag_spirv)) { throw Error{"Failed to create default shader"}; }
	return ret;
}
} // namespace

ResourcePool::ResourcePool(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory,
						   gsl::not_null<ShaderLayout const*> shader_layout)
	: sampler_factory(sampler_factory), shader_layout(shader_layout), m_default_shader(create_default_shader(render_device, *shader_layout)),
	  m_white_texture(render_device, sampler_factory), m_waiter(render_device->get_device()) {}

auto ResourcePool::descriptor_image(ITextureBase const* texture) const -> vk::DescriptorImageInfo {
	return texture ? texture->descriptor_info() : get_white_texture().descriptor_info();
}
} // namespace le::detail
