#include <detail/renderer.hpp>
#include <klib/assert.hpp>
#include <klib/visitor.hpp>
#include <kvf/util.hpp>

namespace le::detail {
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

auto to_viewport(viewport::Letterbox const& v, glm::vec2 const framebuffer_size) {
	auto const world_in_fb_space = v.fill_target_space(framebuffer_size);
	auto const half_excess = 0.5f * (framebuffer_size - world_in_fb_space);
	auto const rect = kvf::Rect<>{.lt = half_excess, .rb = framebuffer_size - half_excess};
	auto const vp_size = rect.size();
	return vk::Viewport{rect.lt.x, rect.rb.y, vp_size.x, -vp_size.y};
}
} // namespace

auto Renderer::begin_render(vk::CommandBuffer const command_buffer, glm::ivec2 size, kvf::Color const clear) -> bool {
	m_stats = {};
	if (!command_buffer || is_rendering() || !m_shader->is_ready()) { return false; }

	size = clamp_size(size);

	m_pass->clear_color = clear.to_linear();
	m_pass->begin_render(command_buffer, kvf::util::to_vk_extent(size));

	return true;
}

auto Renderer::end_render() -> kvf::RenderTarget const& {
	if (is_rendering()) {
		m_rt = m_pass->render_target();
		m_pass->end_render();
	}
	m_pipeline = vk::Pipeline{};
	return m_rt;
}

void Renderer::draw(Primitive const& primitive, std::span<RenderInstance const> instances) {
	auto const cmd = m_pass->get_command_buffer();
	if (!cmd || primitive.vertices.empty() || instances.empty()) { return; }
	if (!bind_shader(primitive.topology)) { return; }

	auto descriptor_sets = std::array<vk::DescriptorSet, 3>{};
	if (!allocate_sets(descriptor_sets)) { return; }

	auto const visitor = klib::Visitor{
		[this](viewport::Dynamic const& v) { return m_pass->to_viewport(v.n_rect); },
		[this](viewport::Letterbox const& v) { return to_viewport(v, framebuffer_size()); },
	};
	m_viewport = std::visit(visitor, viewport);
	m_scissor = m_pass->to_scissor(scissor_rect);

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

	cmd.setViewport(0, m_viewport);
	cmd.setScissor(0, m_scissor);
	cmd.setLineWidth(m_line_width);

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_resource_pool->get_pipeline_layout(), 0, descriptor_sets, {});
	vbo.draw(cmd, std::uint32_t(instances.size()));
	++m_stats.draw_calls;
	m_stats.triangles += triangle_count(primitive.vertices.size(), primitive.indices.size(), primitive.topology);
}

auto Renderer::unprojector() const -> Unprojector { return Unprojector{viewport, view, framebuffer_size()}; }

auto Renderer::bind_shader(vk::PrimitiveTopology const topology) -> bool {
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
	auto const visitor = klib::Visitor{
		[](viewport::Letterbox const& v) { return v.world_size; },
		[this](viewport::Dynamic const& /*v*/) { return glm::vec2{m_viewport.width, -m_viewport.height}; },
	};
	auto const render_area = std::visit(visitor, viewport);
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
} // namespace le::detail
