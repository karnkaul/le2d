#include "detail/renderer.hpp"
#include "klib/debug/assert.hpp"
#include "klib/visitor.hpp"
#include "kvf/render_device.hpp"
#include "kvf/util.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace le::detail {
namespace {
struct Std430Instance {
	glm::mat4 transform;
	glm::vec4 tint;
};

struct Vbo {
	[[nodiscard]] static auto create(kvf::FixedUsageBuffer const& buffer, std::span<Vertex const> vertices, std::span<std::uint32_t const> indices) -> Vbo {
		auto const writes = std::array{
			kvf::BufferWrite{vertices},
			kvf::BufferWrite{indices},
		};
		buffer.write_contiguous(writes);
		return Vbo{
			.buffer = buffer.get_buffer(),
			.vertices = std::uint32_t(vertices.size()),
			.indices = std::uint32_t(indices.size()),
		};
	}

	void draw(vk::CommandBuffer const m_cmd, std::uint32_t const instances) const {
		m_cmd.bindVertexBuffers(0, buffer, vk::DeviceSize{});
		if (indices == 0) {
			m_cmd.draw(vertices, instances, 0, 0);
		} else {
			m_cmd.bindIndexBuffer(buffer, vertices * sizeof(Vertex), vk::IndexType::eUint32);
			m_cmd.drawIndexed(indices, instances, 0, 0, 0);
		}
	}

	vk::Buffer buffer{};
	std::uint32_t vertices{};
	std::uint32_t indices{};
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

auto const scratch_buffer_layout = std::vector<vk::BufferUsageFlags>{
	vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
	vk::BufferUsageFlagBits::eUniformBuffer,
	vk::BufferUsageFlagBits::eStorageBuffer,
	vk::BufferUsageFlagBits::eStorageBuffer,
};
} // namespace

Renderer::Renderer(gsl::not_null<kvf::IRenderPass*> render_pass, gsl::not_null<IRenderResources*> resources)
	: m_render_pass(render_pass), m_resources(resources),
	  m_buffer_allocator(kvf::IRingBufferAllocator::create(&render_pass->get_render_device(), scratch_buffer_layout)),
	  m_descriptor_allocator(&render_pass->get_render_device().get_descriptor_allocator()), m_shader(&resources->get_default_shader()) {}

auto Renderer::begin_render(vk::CommandBuffer const command_buffer, glm::ivec2 size, kvf::Color const clear) -> bool {
	m_stats = {};
	if (!command_buffer || is_rendering()) { return false; }

	size = clamp_size(size);

	m_render_pass->clear_color = clear.to_linear();
	m_render_pass->begin_render(command_buffer, kvf::util::to_vk_extent(size));

	refresh_view_matrices();

	return true;
}

auto Renderer::end_render() -> kvf::RenderTarget const& {
	if (is_rendering()) {
		m_rt = m_render_pass->render_target();
		m_render_pass->end_render();
	}
	return m_rt;
}

void Renderer::set_line_width(float width) {
	width = std::clamp(width, 0.0f, m_render_pass->get_render_device().get_gpu().properties.limits.lineWidthRange[1]);
	m_line_width = width;
}

void Renderer::set_view(Transform const& view) {
	m_view_transform = view;
	refresh_view_matrix();
}

void Renderer::set_viewport(Viewport const& viewport) {
	m_viewport = viewport;
	refresh_projection_matrix();
}

void Renderer::draw(Primitive const& primitive, std::span<RenderInstance const> instances) {
	auto const std430_instances = bake_instances(instances);
	draw_baked(primitive, std430_instances);
}

void Renderer::draw_baked(Primitive const& primitive, std::span<RenderInstance::Std430 const> instances) {
	auto const cmd = m_render_pass->get_command_buffer();
	if (!cmd || primitive.vertices.empty() || instances.empty()) { return; }

	auto descriptor_sets = std::array<vk::DescriptorSet, 3>{};
	if (!allocate_sets(descriptor_sets)) { return; }

	auto const visitor = klib::Visitor{
		[this](viewport::Dynamic const& v) { return m_render_pass->to_viewport(v.n_rect); },
		[this](viewport::Letterbox const& v) { return to_viewport(v, framebuffer_size()); },
	};
	m_vk_viewport = std::visit(visitor, m_viewport);
	m_scissor = m_render_pass->to_scissor(scissor_rect);

	auto& render_device = m_render_pass->get_render_device();

	auto const scratch_buffers = m_buffer_allocator->allocate_next();
	KLIB_ASSERT(scratch_buffers.size() == scratch_buffer_layout.size());

	auto const vbo = Vbo::create(scratch_buffers[0], primitive.vertices, primitive.indices);

	scratch_buffers[1].write(m_view_matrices);
	auto const view_info = scratch_buffers[1].descriptor_info();

	scratch_buffers[2].write(instances);
	auto const instance_info = scratch_buffers[2].descriptor_info();

	auto const texture_info = m_resources->descriptor_image(primitive.texture);

	scratch_buffers[3].write(m_user_data.ssbo);
	auto const user_ssbo_info = scratch_buffers[3].descriptor_info();

	auto const user_texture_info = m_resources->descriptor_image(m_user_data.texture);

	auto const descriptor_writes = std::array{
		kvf::util::ubo_write(&view_info, descriptor_sets[0], 0),		   kvf::util::ssbo_write(&instance_info, descriptor_sets[1], 0),
		kvf::util::image_write(&texture_info, descriptor_sets[1], 1),	   kvf::util::ssbo_write(&user_ssbo_info, descriptor_sets[2], 0),
		kvf::util::image_write(&user_texture_info, descriptor_sets[2], 1),
	};
	render_device.get_device().updateDescriptorSets(descriptor_writes, {});

	m_render_pass->bind_graphics_shader(m_shader->get_kvf_shader());

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_resources->get_shader_layout().get_pipeline_layout(), 0, descriptor_sets, {});

	cmd.setPrimitiveTopology(primitive.topology);
	cmd.setPolygonModeEXT(polygon_mode);

	cmd.setViewport(0, m_vk_viewport);
	cmd.setScissor(0, m_scissor);
	cmd.setLineWidth(m_line_width);

	vbo.draw(cmd, std::uint32_t(instances.size()));
	++m_stats.draw_calls;
	m_stats.triangles += triangle_count(primitive.vertices.size(), primitive.indices.size(), primitive.topology);
}

auto Renderer::unprojector() const -> Unprojector { return Unprojector{m_viewport, m_view_transform, framebuffer_size()}; }

void Renderer::refresh_view_matrices() {
	refresh_view_matrix();
	refresh_projection_matrix();
}

void Renderer::refresh_view_matrix() { m_view_matrices.mat_v = m_view_transform.to_view(); }

void Renderer::refresh_projection_matrix() {
	auto const viewport_visitor = klib::Visitor{
		[this](viewport::Dynamic const& v) { return m_render_pass->to_viewport(v.n_rect); },
		[this](viewport::Letterbox const& v) { return to_viewport(v, framebuffer_size()); },
	};
	m_vk_viewport = std::visit(viewport_visitor, m_viewport);

	auto const render_area_visitor = klib::Visitor{
		[](viewport::Letterbox const& v) { return v.world_size; },
		[this](viewport::Dynamic const& /*v*/) { return glm::vec2{m_vk_viewport.width, -m_vk_viewport.height}; },
	};
	auto const render_area = std::visit(render_area_visitor, m_viewport);
	auto const half_extent = 0.5f * render_area;
	m_view_matrices.mat_p = glm::ortho(-half_extent.x, half_extent.x, -half_extent.y, half_extent.y);
}

auto Renderer::allocate_sets(std::span<vk::DescriptorSet> out_sets) const -> bool {
	auto const set_layouts = m_resources->get_shader_layout().get_set_layouts();
	KLIB_ASSERT(set_layouts.size() == out_sets.size());
	return m_descriptor_allocator->allocate_next(out_sets, set_layouts);
}

auto Renderer::bake_instances(std::span<RenderInstance const> instances) const -> std::span<RenderInstance::Std430 const> {
	m_resources->render_instance_buffer.clear();
	m_resources->render_instance_buffer.reserve(instances.size());
	for (auto const& in : instances) { m_resources->render_instance_buffer.push_back(in.to_std430()); }
	return m_resources->render_instance_buffer;
}
} // namespace le::detail
