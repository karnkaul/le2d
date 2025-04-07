#include <glm/gtc/matrix_transform.hpp>
#include <klib/assert.hpp>
#include <kvf/is_positive.hpp>
#include <kvf/render_device.hpp>
#include <kvf/render_pass.hpp>
#include <kvf/util.hpp>
#include <le2d/renderer.hpp>
#include <le2d/resource_pool.hpp>

namespace le {
namespace {
struct Std430Instance {
	glm::mat4 transform;
	glm::vec4 tint;
};

struct ImageSampler {
	vk::ImageView image{};
	vk::Sampler sampler{};
};

void write_instances(std::vector<std::byte>& out, std::span<RenderInstance const> instances) {
	out.clear();
	out.resize(instances.size() * sizeof(Std430Instance));
	auto write_span = std::span{out};
	for (auto const& in : instances) {
		auto const instance = Std430Instance{
			.transform = in.transform.to_model(),
			.tint = in.tint.to_linear(),
		};
		KLIB_ASSERT(write_span.size() >= sizeof(instance));
		std::memcpy(write_span.data(), &instance, sizeof(instance));
		write_span = write_span.subspan(sizeof(instance));
	}
}

auto buffer_wds(vk::DescriptorBufferInfo const& dbi, vk::DescriptorType type, vk::DescriptorSet set, std::uint32_t binding) {
	auto ret = vk::WriteDescriptorSet{};
	ret.setBufferInfo(dbi).setDescriptorType(type).setDescriptorCount(1).setDstSet(set).setDstBinding(binding);
	return ret;
}

auto image_wds(vk::DescriptorImageInfo const& dii, vk::DescriptorSet set, std::uint32_t binding) {
	auto ret = vk::WriteDescriptorSet{};
	ret.setImageInfo(dii).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setDstSet(set).setDstBinding(binding);
	return ret;
}

constexpr auto triangle_count(std::size_t const vertices, std::size_t const indices, vk::PrimitiveTopology const topology) -> std::int64_t {
	if (vertices == 0) { return 0; }
	auto const target = indices == 0 ? std::int64_t(vertices) : std::int64_t(indices);
	switch (topology) {
	case vk::PrimitiveTopology::eTriangleList: return target / 3;
	case vk::PrimitiveTopology::eTriangleStrip: return target - 2;
	case vk::PrimitiveTopology::eTriangleFan: return target - 2;
	default: return 0;
	}
}
} // namespace

Renderer::Renderer(kvf::RenderPass& render_pass, IResourcePool& resource_pool)
	: m_pass(&render_pass), m_resource_pool(&resource_pool), m_cmd(render_pass.get_command_buffer()), m_shader(&resource_pool.get_default_shader()) {
	if (!m_cmd) {
		m_pass = nullptr;
		return;
	}

	if (!*m_shader || !bind_shader(vk::PrimitiveTopology::eTriangleList)) {
		end_render();
		return;
	}

	m_viewport = render_pass.to_viewport(kvf::uv_rect_v);
	m_scissor = render_pass.to_scissor(kvf::uv_rect_v);
}

auto Renderer::set_line_width(float const width) -> bool {
	if (!is_rendering() || width > m_pass->get_render_device().get_gpu().properties.limits.lineWidthRange[1]) { return false; }

	m_line_width = width;
	return true;
}

auto Renderer::set_shader(Shader const& shader) -> bool {
	if (!is_rendering() || !shader) { return false; }
	if (m_shader == &shader) { return true; }

	m_shader = &shader;
	m_pipeline = vk::Pipeline{};
	return true;
}

auto Renderer::set_render_area(kvf::UvRect const& n_rect) -> bool {
	if (!is_rendering()) { return false; }
	m_viewport = m_pass->to_viewport(n_rect);
	return true;
}

auto Renderer::set_scissor_rect(kvf::UvRect const& n_rect) -> bool {
	if (!is_rendering()) { return false; }
	m_scissor = m_pass->to_scissor(n_rect);
	return true;
}

auto Renderer::set_user_data(UserDrawData const& user_data) -> bool {
	if (!is_rendering()) { return false; }

	m_user_data = user_data;
	return true;
}

auto Renderer::draw(Primitive const& primitive, std::span<RenderInstance const> instances) -> bool {
	if (!is_rendering()) { return false; }
	if (primitive.vertices.empty() || instances.empty()) { return true; }

	if (!bind_shader(primitive.topology)) { return false; }

	auto& render_device = m_pass->get_render_device();

	auto descriptor_sets = std::array<vk::DescriptorSet, 3>{};
	auto const set_layouts = m_resource_pool->get_set_layouts();
	KLIB_ASSERT(set_layouts.size() == descriptor_sets.size());
	if (!render_device.allocate_sets(descriptor_sets, set_layouts)) { return false; }

	auto const vbo_size = primitive.vertices.size_bytes() + primitive.indices.size_bytes();
	auto const vbo_writes = std::array{
		kvf::BufferWrite{primitive.vertices},
		kvf::BufferWrite{primitive.indices},
	};
	auto& vbo = render_device.allocate_scratch_buffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer, vbo_size);
	vbo.overwrite_contiguous(vbo_writes);

	auto const render_area = glm::vec2{m_viewport.width, -m_viewport.height};
	auto const half_extent = 0.5f * render_area;
	auto const mat_p = glm::ortho(-half_extent.x, half_extent.x, -half_extent.y, half_extent.y);
	auto const mat_v = view.to_view();
	auto const mat_vp = mat_p * mat_v;
	auto const view_info = render_device.scratch_descriptor_buffer(vk::BufferUsageFlagBits::eUniformBuffer, mat_vp);

	write_instances(m_resource_pool->scratch_buffer, instances);
	auto const instance_info = render_device.scratch_descriptor_buffer(vk::BufferUsageFlagBits::eStorageBuffer, m_resource_pool->scratch_buffer);

	auto const user_ssbo_info = render_device.scratch_descriptor_buffer(vk::BufferUsageFlagBits::eStorageBuffer, m_user_data.ssbo);

	auto const texture_info = m_resource_pool->descriptor_image(primitive.texture);
	auto const user_texture_info = m_resource_pool->descriptor_image(m_user_data.texture);

	auto const descriptor_writes = std::array{
		buffer_wds(view_info, vk::DescriptorType::eUniformBuffer, descriptor_sets[0], 0),
		buffer_wds(instance_info, vk::DescriptorType::eStorageBuffer, descriptor_sets[1], 0),
		image_wds(texture_info, descriptor_sets[1], 1),
		buffer_wds(user_ssbo_info, vk::DescriptorType::eStorageBuffer, descriptor_sets[2], 0),
		image_wds(user_texture_info, descriptor_sets[2], 1),
	};
	render_device.get_device().updateDescriptorSets(descriptor_writes, {});

	m_cmd.setViewport(0, m_viewport);
	m_cmd.setScissor(0, m_scissor);
	m_cmd.setLineWidth(m_line_width);

	m_cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_resource_pool->get_pipeline_layout(), 0, descriptor_sets, {});
	m_cmd.bindVertexBuffers(0, vbo.get_buffer(), vk::DeviceSize{});
	if (primitive.indices.empty()) {
		auto const vertices = std::uint32_t(primitive.vertices.size());
		m_cmd.draw(vertices, std::uint32_t(instances.size()), 0, 0);
	} else {
		auto const indices = std::uint32_t(primitive.indices.size());
		m_cmd.bindIndexBuffer(vbo.get_buffer(), primitive.vertices.size_bytes(), vk::IndexType::eUint32);
		m_cmd.drawIndexed(indices, std::uint32_t(instances.size()), 0, 0, 0);
	}
	++m_stats.draw_calls;
	m_stats.triangles += triangle_count(primitive.vertices.size(), primitive.indices.size(), primitive.topology);

	return true;
}

auto Renderer::end_render() -> kvf::RenderTarget {
	if (!is_rendering()) { return {}; }

	m_pass->end_render();
	auto const& ret = m_pass->render_target();
	m_pass = nullptr;
	return ret;
}

auto Renderer::bind_shader(vk::PrimitiveTopology const topology) -> bool {
	if (!is_rendering()) { return false; }

	KLIB_ASSERT(m_shader != nullptr);

	auto const fixed_state = PipelineFixedState{
		.samples = m_pass->get_samples(),
		.topology = topology,
		.polygon_mode = polygon_mode,
	};
	auto const pipeline = m_resource_pool->allocate_pipeline(fixed_state, *m_shader);
	if (!pipeline) { return false; }

	if (m_pipeline == pipeline) { return true; }

	m_pass->bind_pipeline(pipeline);
	m_pipeline = pipeline;

	return true;
}
} // namespace le
