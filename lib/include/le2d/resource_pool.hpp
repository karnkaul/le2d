#pragma once
#include <le2d/pipeline_fixed_state.hpp>
#include <le2d/resource/shader.hpp>
#include <le2d/resource/texture.hpp>
#include <cstddef>
#include <vector>

namespace le {
class IResourcePool : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto allocate_pipeline(PipelineFixedState const& state, IShader const& shader) -> vk::Pipeline = 0;

	[[nodiscard]] virtual auto get_pipeline_layout() const -> vk::PipelineLayout = 0;
	[[nodiscard]] virtual auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const> = 0;

	[[nodiscard]] virtual auto get_white_texture() const -> ITexture const& = 0;
	[[nodiscard]] virtual auto get_default_shader() const -> IShader const& = 0;

	[[nodiscard]] auto descriptor_image(ITextureBase const* texture) const -> vk::DescriptorImageInfo {
		return texture && texture->is_ready() ? texture->descriptor_info() : get_white_texture().descriptor_info();
	}

	std::vector<std::byte> scratch_buffer{};
};
} // namespace le
