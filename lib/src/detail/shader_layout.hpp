#pragma once
#include <vulkan/vulkan.hpp>
#include <array>
#include <ranges>

namespace le::detail {
class ShaderLayout {
  public:
	static constexpr auto set_count_v{3uz};

	explicit ShaderLayout(vk::Device const device) {
		create_set_layouts(device);
		create_pipeline_layout(device);
	}

	[[nodiscard]] auto get_pipeline_layout() const -> vk::PipelineLayout { return *m_pipeline_layout; }
	[[nodiscard]] auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const, set_count_v> { return m_set_layouts; }

  private:
	void create_set_layouts(vk::Device const device) {
		static constexpr auto set_0_bindings_v = std::array{
			vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics},
		};
		static constexpr auto set_1_bindings_v = std::array{
			vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics},
			vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eAllGraphics},
		};
		static constexpr auto set_2_bindings = set_1_bindings_v;

		auto dslci = vk::DescriptorSetLayoutCreateInfo{};
		dslci.setBindings(set_0_bindings_v);
		m_set_layout_storage[0] = device.createDescriptorSetLayoutUnique(dslci);

		dslci.setBindings(set_1_bindings_v);
		m_set_layout_storage[1] = device.createDescriptorSetLayoutUnique(dslci);

		dslci.setBindings(set_2_bindings);
		m_set_layout_storage[2] = device.createDescriptorSetLayoutUnique(dslci);

		for (auto [in, out] : std::ranges::zip_view(m_set_layout_storage, m_set_layouts)) { out = *in; }
	}

	void create_pipeline_layout(vk::Device const device) {
		auto plci = vk::PipelineLayoutCreateInfo{};
		plci.setSetLayouts(m_set_layouts);
		m_pipeline_layout = device.createPipelineLayoutUnique(plci);
	}

	std::array<vk::UniqueDescriptorSetLayout, set_count_v> m_set_layout_storage{};
	std::array<vk::DescriptorSetLayout, set_count_v> m_set_layouts{};
	vk::UniquePipelineLayout m_pipeline_layout{};
};
} // namespace le::detail
