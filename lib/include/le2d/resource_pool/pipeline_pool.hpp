#pragma once
#include <kvf/device_block.hpp>
#include <kvf/render_device_fwd.hpp>
#include <le2d/shader.hpp>
#include <vulkan/vulkan.hpp>
#include <array>
#include <gsl/pointers>
#include <unordered_map>

namespace le {
class PipelinePool {
  public:
	static constexpr std::size_t set_count_v{3};

	struct Info {
		vk::SampleCountFlagBits samples;
		vk::PrimitiveTopology topology;
		vk::PolygonMode polygon_mode;
	};

	explicit PipelinePool(gsl::not_null<kvf::RenderDevice const*> render_device);

	[[nodiscard]] auto get_layout() const -> vk::PipelineLayout { return *m_layout; }
	[[nodiscard]] auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const, set_count_v> { return m_set_layouts; }

	[[nodiscard]] auto allocate(Info const& info, Shader const& shader) -> vk::Pipeline;

  private:
	struct Entry {
		std::size_t shader_hash;
		vk::SampleCountFlagBits samples;
		vk::PrimitiveTopology topology;
		vk::PolygonMode polygon_mode;

		auto operator==(Entry const&) const -> bool = default;
	};

	struct Hasher {
		[[nodiscard]] auto operator()(Entry const& entry) const -> std::size_t;
	};

	void create_set_layouts();
	void create_layout();

	[[nodiscard]] auto create(Info const& info, Shader const& shader) const -> vk::UniquePipeline;

	gsl::not_null<kvf::RenderDevice const*> m_render_device;

	vk::UniquePipelineLayout m_layout{};
	std::array<vk::UniqueDescriptorSetLayout, set_count_v> m_set_layout_storage{};
	std::array<vk::DescriptorSetLayout, set_count_v> m_set_layouts{};

	std::unordered_map<Entry, vk::UniquePipeline, Hasher> m_map{};

	kvf::DeviceBlock m_blocker;
};
} // namespace le
