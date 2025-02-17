#pragma once
#include <le2d/pipeline_fixed_state.hpp>
#include <le2d/shader.hpp>
#include <le2d/texture.hpp>
#include <cstddef>
#include <vector>

namespace le {
class IResourcePool : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto allocate_buffer(vk::BufferUsageFlags usage, vk::DeviceSize size) -> kvf::vma::Buffer& = 0;
	[[nodiscard]] virtual auto allocate_pipeline(PipelineFixedState const& state, Shader const& shader) -> vk::Pipeline = 0;
	[[nodiscard]] virtual auto allocate_sampler(TextureSampler const& sampler) -> vk::Sampler = 0;

	[[nodiscard]] virtual auto get_pipeline_layout() const -> vk::PipelineLayout = 0;
	[[nodiscard]] virtual auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const> = 0;

	[[nodiscard]] virtual auto get_white_texture() const -> Texture const& = 0;
	[[nodiscard]] virtual auto get_default_shader() const -> Shader const& = 0;

	std::vector<std::byte> scratch_buffer{};
};
} // namespace le
