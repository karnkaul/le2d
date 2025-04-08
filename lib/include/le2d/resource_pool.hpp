#pragma once
#include <le2d/pipeline_fixed_state.hpp>
#include <le2d/shader_program.hpp>
#include <le2d/texture.hpp>
#include <cstddef>
#include <vector>

namespace le {
class IResourcePool : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto allocate_pipeline(PipelineFixedState const& state, ShaderProgram const& shader) -> vk::Pipeline = 0;

	[[nodiscard]] virtual auto get_pipeline_layout() const -> vk::PipelineLayout = 0;
	[[nodiscard]] virtual auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const> = 0;

	[[nodiscard]] virtual auto get_white_texture() const -> Texture const& = 0;
	[[nodiscard]] virtual auto get_default_shader() const -> ShaderProgram const& = 0;

	[[nodiscard]] auto descriptor_image(ITexture const* texture) const -> vk::DescriptorImageInfo {
		return texture != nullptr ? texture->descriptor_info() : get_white_texture().descriptor_info();
	}

	std::vector<std::byte> scratch_buffer{};
};
} // namespace le
