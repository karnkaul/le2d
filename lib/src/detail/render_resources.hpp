#pragma once
#include "detail/shader_layout.hpp"
#include "klib/base_types.hpp"
#include "le2d/render_instance.hpp"
#include "le2d/resource/shader.hpp"
#include "le2d/resource/texture.hpp"
#include <vector>

namespace le::detail {
class ISamplerFactory : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_render_device() const -> kvf::IRenderDevice& = 0;
	[[nodiscard]] virtual auto get_or_create(TextureSampler const& sampler) -> vk::Sampler = 0;
};

class IRenderResources : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_shader_layout() const -> ShaderLayout const& = 0;
	[[nodiscard]] virtual auto get_white_texture() const -> ITexture const& = 0;
	[[nodiscard]] virtual auto get_default_shader() const -> IShader const& = 0;

	[[nodiscard]] auto descriptor_image(ITextureBase const* texture) const -> vk::DescriptorImageInfo {
		return texture ? texture->descriptor_info() : get_white_texture().descriptor_info();
	}

	std::vector<RenderInstance::Std430> render_instance_buffer{};
};
} // namespace le::detail
