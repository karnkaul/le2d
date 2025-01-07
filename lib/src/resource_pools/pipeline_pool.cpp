#include <klib/hash_combine.hpp>
#include <kvf/render_device.hpp>
#include <kvf/util.hpp>
#include <le2d/resource_pools/pipeline_pool.hpp>
#include <le2d/vertex.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <ranges>

namespace le {
auto PipelinePool::Hasher::operator()(Entry const& entry) const -> std::size_t {
	return klib::make_combined_hash(entry.shader_hash, entry.samples, entry.topology, entry.polygon_mode);
}

PipelinePool::PipelinePool(gsl::not_null<kvf::RenderDevice const*> render_device) : m_render_device(render_device), m_blocker(render_device->get_device()) {
	create_set_layouts();
	create_layout();
}

auto PipelinePool::allocate(Info const& info, Shader const& shader) -> vk::Pipeline {
	auto const entry = Entry{
		.shader_hash = shader.get_hash(),
		.samples = info.samples,
		.topology = info.topology,
		.polygon_mode = info.polygon_mode,
	};
	auto it = m_map.find(entry);
	if (it == m_map.end()) {
		auto pipeline = create(info, shader);
		if (!pipeline) { return {}; }
		it = m_map.insert({entry, std::move(pipeline)}).first;
	}
	return *it->second;
}

void PipelinePool::create_set_layouts() {
	auto const set_0_bindings = std::array{
		vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics},
	};
	auto const set_1_bindings = std::array{
		vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics},
		vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eAllGraphics},
	};
	auto const set_2_bindings = set_1_bindings;

	auto dslci = vk::DescriptorSetLayoutCreateInfo{};
	dslci.setBindings(set_0_bindings);
	m_set_layout_storage[0] = m_render_device->get_device().createDescriptorSetLayoutUnique(dslci);

	dslci.setBindings(set_1_bindings);
	m_set_layout_storage[1] = m_render_device->get_device().createDescriptorSetLayoutUnique(dslci);

	dslci.setBindings(set_2_bindings);
	m_set_layout_storage[2] = m_render_device->get_device().createDescriptorSetLayoutUnique(dslci);

	for (auto [in, out] : std::ranges::zip_view(m_set_layout_storage, m_set_layouts)) { out = *in; }
}

void PipelinePool::create_layout() {
	auto plci = vk::PipelineLayoutCreateInfo{};
	plci.setSetLayouts(m_set_layouts);
	m_layout = m_render_device->get_device().createPipelineLayoutUnique(plci);
}

auto PipelinePool::create(Info const& info, Shader const& shader) const -> vk::UniquePipeline {
	static constexpr auto bindings_v = std::array{
		vk::VertexInputBindingDescription{0, sizeof(Vertex)},
	};

	static constexpr auto attributes_v = std::array{
		vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)},
		vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
		vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
	};

	auto const modules = shader.get_modules();
	auto const state = kvf::PipelineState{
		.vertex_bindings = bindings_v,
		.vertex_attributes = attributes_v,
		.vertex_shader = modules.vertex,
		.fragment_shader = modules.fragment,
		.topology = info.topology,
		.polygon_mode = info.polygon_mode,
		.cull_mode = vk::CullModeFlagBits::eNone,
		.depth_compare = vk::CompareOp::eNever,
		.flags = kvf::PipelineFlag::AlphaBlend,
	};
	auto const format = kvf::PipelineFormat{
		.samples = info.samples,
		.color = vk::Format::eR8G8B8A8Srgb,
	};
	return m_render_device->create_pipeline(*m_layout, state, format);
}
} // namespace le
