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

struct Vbo {
	explicit Vbo(kvf::RenderDevice& render_device, std::span<Vertex const> vertices, std::span<std::uint32_t const> indices)
		: m_buffer(render_device.allocate_scratch_buffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
														 vertices.size_bytes() + indices.size_bytes())),
		  m_vertices(std::uint32_t(vertices.size())), m_indices(std::uint32_t(indices.size())) {
		auto const writes = std::array{
			kvf::BufferWrite{vertices},
			kvf::BufferWrite{indices},
		};
		m_buffer.overwrite_contiguous(writes);
	}

	void draw(vk::CommandBuffer const m_cmd, std::uint32_t const instances) const {
		m_cmd.bindVertexBuffers(0, m_buffer.get_buffer(), vk::DeviceSize{});
		if (m_indices == 0) {
			m_cmd.draw(m_vertices, instances, 0, 0);
		} else {
			m_cmd.bindIndexBuffer(m_buffer.get_buffer(), m_vertices * sizeof(Vertex), vk::IndexType::eUint32);
			m_cmd.drawIndexed(m_indices, instances, 0, 0, 0);
		}
	}

  private:
	kvf::vma::Buffer& m_buffer;
	std::uint32_t m_vertices{};
	std::uint32_t m_indices{};
};

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

	if (!m_shader->is_ready() || !bind_shader(vk::PrimitiveTopology::eTriangleList)) {
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

auto Renderer::set_shader(IShader const& shader) -> bool {
	if (!is_rendering() || !shader.is_ready()) { return false; }
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

	auto descriptor_sets = std::array<vk::DescriptorSet, 3>{};
	if (!allocate_sets(descriptor_sets)) { return false; }

	auto& render_device = m_pass->get_render_device();

	auto const vbo = Vbo{render_device, primitive.vertices, primitive.indices};
	auto const view_info = write_view();
	auto const instance_info = write_instances(instances);
	auto const texture_info = m_resource_pool->descriptor_image(primitive.texture);

	auto const user_ssbo_info = render_device.scratch_descriptor_buffer(vk::BufferUsageFlagBits::eStorageBuffer, m_user_data.ssbo);
	auto const user_texture_info = m_resource_pool->descriptor_image(m_user_data.texture);

	auto const descriptor_writes = std::array{
		kvf::util::ubo_write(&view_info, descriptor_sets[0], 0),		   kvf::util::ssbo_write(&instance_info, descriptor_sets[1], 0),
		kvf::util::image_write(&texture_info, descriptor_sets[1], 1),	   kvf::util::ssbo_write(&user_ssbo_info, descriptor_sets[2], 0),
		kvf::util::image_write(&user_texture_info, descriptor_sets[2], 1),
	};
	render_device.get_device().updateDescriptorSets(descriptor_writes, {});

	m_cmd.setViewport(0, m_viewport);
	m_cmd.setScissor(0, m_scissor);
	m_cmd.setLineWidth(m_line_width);

	m_cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_resource_pool->get_pipeline_layout(), 0, descriptor_sets, {});
	vbo.draw(m_cmd, std::uint32_t(instances.size()));
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

auto Renderer::allocate_sets(std::span<vk::DescriptorSet> out_sets) const -> bool {
	auto const set_layouts = m_resource_pool->get_set_layouts();
	KLIB_ASSERT(set_layouts.size() == out_sets.size());
	return m_pass->get_render_device().allocate_sets(out_sets, set_layouts);
}

auto Renderer::write_view() const -> vk::DescriptorBufferInfo {
	auto const render_area = glm::vec2{m_viewport.width, -m_viewport.height};
	auto const half_extent = 0.5f * render_area;
	auto const mat_p = glm::ortho(-half_extent.x, half_extent.x, -half_extent.y, half_extent.y);
	auto const mat_v = view.to_view();
	auto const mat_vp = mat_p * mat_v;
	return m_pass->get_render_device().scratch_descriptor_buffer(vk::BufferUsageFlagBits::eUniformBuffer, mat_vp);
}

auto Renderer::write_instances(std::span<RenderInstance const> instances) const -> vk::DescriptorBufferInfo {
	m_resource_pool->scratch_buffer.clear();
	m_resource_pool->scratch_buffer.resize(instances.size() * sizeof(Std430Instance));
	auto write_span = std::span{m_resource_pool->scratch_buffer};
	for (auto const& in : instances) {
		auto const instance = Std430Instance{
			.transform = in.transform.to_model(),
			.tint = in.tint.to_linear(),
		};
		KLIB_ASSERT(write_span.size() >= sizeof(instance));
		std::memcpy(write_span.data(), &instance, sizeof(instance));
		write_span = write_span.subspan(sizeof(instance));
	}
	return m_pass->get_render_device().scratch_descriptor_buffer(vk::BufferUsageFlagBits::eStorageBuffer, m_resource_pool->scratch_buffer);
}
} // namespace le
