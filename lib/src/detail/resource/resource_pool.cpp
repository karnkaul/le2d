#include "detail/resource/resource_pool.hpp"
#include "le2d/error.hpp"
#include "spirv.hpp"

namespace le::detail {
namespace {
[[nodiscard]] auto create_default_shader(gsl::not_null<IResourceFactory const*> resource_factory) -> std::unique_ptr<IShader> {
	auto const vert_spirv = spirv::vert();
	auto const frag_spirv = spirv::frag();
	auto ret = resource_factory->create_shader(vert_spirv, frag_spirv);
	if (!ret->load(vert_spirv, frag_spirv)) { throw Error{"Failed to create default shader"}; }
	return ret;
}
} // namespace

ResourcePool::ResourcePool(gsl::not_null<ResourceFactory const*> resource_factory)
	: sampler_factory(&resource_factory->get_sampler_factory()), shader_layout(&resource_factory->get_shader_layout()),
	  m_default_shader(create_default_shader(resource_factory)), m_white_texture(&resource_factory->get_render_device(), sampler_factory),
	  m_waiter(resource_factory->get_render_device().get_device()) {}

auto ResourcePool::descriptor_image(ITextureBase const* texture) const -> vk::DescriptorImageInfo {
	return texture ? texture->descriptor_info() : get_white_texture().descriptor_info();
}
} // namespace le::detail
