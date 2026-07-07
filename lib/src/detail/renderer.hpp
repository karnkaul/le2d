#pragma once
#include "detail/resource/resource_pool.hpp"
#include "kvf/render_device.hpp"
#include "kvf/render_pass.hpp"
#include "kvf/ring_buffer_allocator.hpp"
#include "kvf/util.hpp"
#include "le2d/renderer.hpp"
#include <algorithm>

namespace le::detail {
class Renderer : public IRenderer {
  public:
	explicit Renderer(gsl::not_null<kvf::IRenderPass*> render_pass, gsl::not_null<detail::ResourcePool*> resource_pool);

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
	[[nodiscard]] auto get_view() const -> Transform const& final { return m_view_transform; }
	void set_view(Transform const& view) final;
	[[nodiscard]] auto get_viewport() const -> Viewport const& final { return m_viewport; }
	void set_viewport(Viewport const& viewport) final;

	void draw(Primitive const& primitive, std::span<RenderInstance const> instances) final;
	void draw_baked(Primitive const& primitive, std::span<RenderInstance::Std430 const> instances) final;

	[[nodiscard]] auto unprojector() const -> Unprojector final;

	void refresh_mat_vp();

	[[nodiscard]] auto allocate_sets(std::span<vk::DescriptorSet> out_sets) const -> bool;

	[[nodiscard]] auto bake_instances(std::span<RenderInstance const> instances) const -> std::span<RenderInstance::Std430 const>;

	gsl::not_null<kvf::IRenderPass*> m_pass;
	gsl::not_null<detail::ResourcePool*> m_resource_pool;
	std::shared_ptr<kvf::IRingBufferAllocator> m_buffer_allocator;
	gsl::not_null<kvf::IRingDescriptorAllocator*> m_descriptor_allocator;

	gsl::not_null<IShader const*> m_shader;
	Transform m_view_transform{};
	glm::mat4 m_mat_vp{1.0f};
	Viewport m_viewport{viewport::Dynamic{}};
	vk::Viewport m_vk_viewport{};
	vk::Rect2D m_scissor{};
	UserDrawData m_user_data{};

	float m_line_width{1.0f};

	kvf::RenderTarget m_rt{};
	RenderStats m_stats{};
};
} // namespace le::detail
