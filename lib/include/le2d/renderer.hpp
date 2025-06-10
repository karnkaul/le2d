#pragma once
#include <kvf/rect.hpp>
#include <kvf/render_target.hpp>
#include <le2d/primitive.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/render_stats.hpp>
#include <le2d/resource_pool.hpp>
#include <le2d/shader_program.hpp>
#include <le2d/user_draw_data.hpp>

namespace kvf {
class RenderPass;
} // namespace kvf

namespace le {
class Renderer {
  public:
	using UserData = UserDrawData;

	Renderer(Renderer const&) = delete;
	Renderer(Renderer&&) = delete;
	auto operator=(Renderer const&) = delete;
	auto operator=(Renderer&&) = delete;

	Renderer() = default;

	/// \param render_pass RenderPass instance. Rendering must have already begun.
	/// \param resource_pool ResourcePool instance.
	explicit Renderer(kvf::RenderPass& render_pass, IResourcePool& resource_pool);
	~Renderer() { end_render(); }

	auto set_line_width(float width) -> bool;
	auto set_shader(ShaderProgram const& shader) -> bool;
	auto set_render_area(kvf::UvRect const& n_rect) -> bool;
	auto set_scissor_rect(kvf::UvRect const& n_rect) -> bool;
	auto set_user_data(UserDrawData const& user_data) -> bool;

	/// \brief Draw given instances of a Primitive.
	/// \param primitive Primitive to draw.
	/// \param instances Render Instances to draw.
	auto draw(Primitive const& primitive, std::span<RenderInstance const> instances) -> bool;

	/// \returns true if rendering is taking place.
	[[nodiscard]] auto is_rendering() const -> bool { return m_pass != nullptr; }
	/// \returns RenderTarget ready to be sampled or presented.
	auto end_render() -> kvf::RenderTarget;

	[[nodiscard]] auto command_buffer() const -> vk::CommandBuffer { return m_cmd; }
	[[nodiscard]] auto get_stats() const -> RenderStats const& { return m_stats; }

	explicit operator bool() const { return is_rendering(); }

	/// \brief Render view (generates view matrix).
	Transform view{};
	/// \brief Fill mode.
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};

  private:
	auto bind_shader(vk::PrimitiveTopology topology) -> bool;

	[[nodiscard]] auto allocate_sets(std::span<vk::DescriptorSet> out_sets) const -> bool;
	[[nodiscard]] auto write_view() const -> vk::DescriptorBufferInfo;
	[[nodiscard]] auto write_instances(std::span<RenderInstance const> instances) const -> vk::DescriptorBufferInfo;

	kvf::RenderPass* m_pass{};
	IResourcePool* m_resource_pool{};
	vk::CommandBuffer m_cmd{};

	ShaderProgram const* m_shader{};
	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
	UserDrawData m_user_data{};

	vk::Pipeline m_pipeline{};
	float m_line_width{1.0f};

	RenderStats m_stats{};
};
} // namespace le
