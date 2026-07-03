#pragma once
#include "kvf/render_device.hpp"
#include "kvf/render_pass.hpp"
#include "kvf/scratch_buffer.hpp"
#include "kvf/util.hpp"
#include "le2d/renderer.hpp"
#include <detail/resource/resource_pool.hpp>
#include <algorithm>

namespace le::detail {
class Renderer : public IRenderer {
  public:
	explicit Renderer(gsl::not_null<kvf::RenderPass*> render_pass, gsl::not_null<detail::ResourcePool*> resource_pool);

  private:
	static constexpr auto clamp_size(glm::ivec2 in) {
		in.x = std::clamp(in.x, min_size_v, max_size_v);
		in.y = std::clamp(in.y, min_size_v, max_size_v);
		return in;
	}

	[[nodiscard]] auto command_buffer() const -> vk::CommandBuffer final { return m_pass->get_command_buffer(); }
	[[nodiscard]] auto get_stats() const -> RenderStats const& final { return m_stats; }

	auto begin_render(vk::CommandBuffer command_buffer, glm::ivec2 size, kvf::Color clear) -> bool final;
	auto end_render() -> kvf::RenderTarget const& final;

	void set_line_width(float width) final {
		width = std::clamp(width, 0.0f, m_pass->get_render_device().get_gpu().properties.limits.lineWidthRange[1]);
		m_line_width = width;
	}

	void set_shader(IShader const& shader) final { m_shader = &shader; }
	void set_user_data(UserDrawData const& user_data) final { m_user_data = user_data; }

	[[nodiscard]] auto framebuffer_size() const -> glm::ivec2 final { return kvf::util::to_glm_vec<int>(m_pass->get_extent()); }

	void draw(Primitive const& primitive, std::span<RenderInstance const> instances) final;

	[[nodiscard]] auto unprojector() const -> Unprojector final;

	auto bind_shader(vk::PrimitiveTopology topology) -> bool;

	[[nodiscard]] auto allocate_sets(std::span<vk::DescriptorSet> out_sets) const -> bool;
	[[nodiscard]] auto write_view_to(kvf::ScratchBuffer& buffer) const -> vk::DescriptorBufferInfo;
	[[nodiscard]] auto write_instances_to(kvf::ScratchBuffer& buffer, std::span<RenderInstance const> instances) const -> vk::DescriptorBufferInfo;

	gsl::not_null<kvf::RenderPass*> m_pass;
	gsl::not_null<detail::ResourcePool*> m_resource_pool;
	kvf::ScratchBuffer::Allocator m_scratch_buffers;

	gsl::not_null<IShader const*> m_shader;
	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
	UserDrawData m_user_data{};

	vk::Pipeline m_pipeline{};
	float m_line_width{1.0f};

	kvf::RenderTarget m_rt{};
	RenderStats m_stats{};
};
} // namespace le::detail
