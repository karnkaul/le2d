#include <glm/gtc/matrix_transform.hpp>
#include <klib/assert.hpp>
#include <kvf/render_device.hpp>
#include <kvf/util.hpp>
#include <le2d/render_pass.hpp>
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

auto write_view_proj(kvf::vma::Buffer& out, Transform const& view, glm::vec2 const render_area) {
	auto const half_extent = 0.5f * render_area;
	auto const mat_p = glm::ortho(-half_extent.x, half_extent.x, -half_extent.y, half_extent.y);
	auto const mat_v = view.to_view();
	auto const mat_vp = mat_p * mat_v;
	return kvf::util::overwrite(out, mat_vp);
}

auto get_image_sampler(ResourcePool& resource_pool, ITexture const* texture) -> ImageSampler {
	if (texture == nullptr) { texture = &resource_pool.white_texture; }
	return ImageSampler{
		.image = texture->get_image(),
		.sampler = resource_pool.samplers.allocate(texture->sampler),
	};
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
} // namespace

Renderer::Renderer(RenderPass& render_pass, ResourcePool& resource_pool, vk::CommandBuffer const command_buffer)
	: m_pass(&render_pass), m_resource_pool(&resource_pool), m_cmd(command_buffer), m_shader(&m_resource_pool->default_shader),
	  m_viewport(render_pass.m_render_pass.to_viewport(kvf::uv_rect_v)), m_scissor(render_pass.m_render_pass.to_scissor(kvf::uv_rect_v)) {
	if (!*m_shader || !bind_shader(vk::PrimitiveTopology::eTriangleList)) { end_render(); }
}

auto Renderer::set_line_width(float const width) -> bool {
	if (!is_rendering() || width > m_pass->get_render_device().get_gpu().properties.limits.lineWidthRange[1]) { return false; }

	m_line_width = width;
	return true;
}

auto Renderer::set_shader(Shader const& shader) -> bool {
	if (!is_rendering() || !shader) { return false; }

	m_shader = &shader;
	m_pipeline = vk::Pipeline{};
	return true;
}

auto Renderer::set_render_area(kvf::UvRect const& n_rect) -> bool {
	if (!is_rendering()) { return false; }
	m_viewport = m_pass->m_render_pass.to_viewport(n_rect);
	return true;
}

auto Renderer::set_scissor_rect(kvf::UvRect const& n_rect) -> bool {
	if (!is_rendering()) { return false; }
	m_scissor = m_pass->m_render_pass.to_scissor(n_rect);
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

	auto descriptor_sets = std::array<vk::DescriptorSet, PipelinePool::set_count_v>{};
	auto const set_layouts = m_resource_pool->pipelines.get_set_layouts();
	if (!m_pass->m_render_device->allocate_sets(descriptor_sets, set_layouts)) { return false; }

	auto const vbo_size = primitive.vertices.size_bytes() + primitive.indices.size_bytes();
	auto& vbo = m_resource_pool->buffers.allocate(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer, vbo_size);
	if (!kvf::util::overwrite(vbo, primitive.vertices)) { return false; }
	if (!primitive.indices.empty() && !kvf::util::overwrite(vbo, primitive.indices, primitive.vertices.size_bytes())) { return false; }

	auto& view_buffer = m_resource_pool->buffers.allocate(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(glm::mat4));
	auto const render_area = glm::vec2{m_viewport.width, -m_viewport.height};
	if (!write_view_proj(view_buffer, view, render_area)) { return false; }

	write_instances(m_resource_pool->scratch_buffer, instances);
	auto& instances_buffer = m_resource_pool->buffers.allocate(vk::BufferUsageFlagBits::eStorageBuffer, m_resource_pool->scratch_buffer.size());
	if (!kvf::util::overwrite(instances_buffer, m_resource_pool->scratch_buffer)) { return false; }

	auto& user_ssbo = m_resource_pool->buffers.allocate(vk::BufferUsageFlagBits::eStorageBuffer, m_user_data.ssbo.size());
	kvf::util::overwrite(user_ssbo, m_user_data.ssbo);

	auto const texture = get_image_sampler(*m_resource_pool, primitive.texture);
	auto const user_texture = get_image_sampler(*m_resource_pool, m_user_data.texture);

	auto dbis = std::array<vk::DescriptorBufferInfo, 3>{};
	auto diis = std::array<vk::DescriptorImageInfo, 2>{};
	dbis[0].setBuffer(view_buffer.get_buffer()).setRange(view_buffer.get_size());
	dbis[1].setBuffer(instances_buffer.get_buffer()).setRange(instances_buffer.get_size());
	dbis[2].setBuffer(user_ssbo.get_buffer()).setRange(user_ssbo.get_size());
	diis[0].setImageView(texture.image).setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setSampler(texture.sampler);
	diis[1].setImageView(user_texture.image).setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setSampler(user_texture.sampler);

	auto const descriptor_writes = std::array{
		buffer_wds(dbis[0], vk::DescriptorType::eUniformBuffer, descriptor_sets[0], 0),
		buffer_wds(dbis[1], vk::DescriptorType::eStorageBuffer, descriptor_sets[1], 0),
		image_wds(diis[0], descriptor_sets[1], 1),
		buffer_wds(dbis[2], vk::DescriptorType::eStorageBuffer, descriptor_sets[2], 0),
		image_wds(diis[1], descriptor_sets[2], 1),
	};
	m_pass->m_render_device->get_device().updateDescriptorSets(descriptor_writes, {});

	m_cmd.setViewport(0, m_viewport);
	m_cmd.setScissor(0, m_scissor);
	m_cmd.setLineWidth(m_line_width);

	m_cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_resource_pool->pipelines.get_layout(), 0, descriptor_sets, {});
	m_cmd.bindVertexBuffers(0, vbo.get_buffer(), vk::DeviceSize{});
	if (primitive.indices.empty()) {
		auto const vertices = std::uint32_t(primitive.vertices.size());
		m_cmd.draw(vertices, std::uint32_t(instances.size()), 0, 0);
	} else {
		auto const indices = std::uint32_t(primitive.indices.size());
		m_cmd.bindIndexBuffer(vbo.get_buffer(), primitive.vertices.size_bytes(), vk::IndexType::eUint32);
		m_cmd.drawIndexed(indices, std::uint32_t(instances.size()), 0, 0, 0);
	}
	return true;
}

auto Renderer::end_render() -> kvf::RenderTarget {
	if (!is_rendering()) { return {}; }

	m_pass->m_render_pass.end_render();
	auto const& ret = m_pass->m_render_pass.render_target();
	m_pass = nullptr;
	return ret;
}

auto Renderer::bind_shader(vk::PrimitiveTopology const topology) -> bool {
	if (!is_rendering()) { return false; }

	KLIB_ASSERT(m_shader != nullptr);

	auto const info = PipelinePool::Info{
		.samples = m_pass->get_samples(),
		.topology = topology,
		.polygon_mode = polygon_mode,
	};
	auto const pipeline = m_resource_pool->pipelines.allocate(info, *m_shader);
	if (!pipeline) { return false; }

	if (m_pipeline == pipeline) { return true; }

	m_pass->m_render_pass.bind_pipeline(pipeline);
	m_pipeline = pipeline;

	return true;
}
} // namespace le
