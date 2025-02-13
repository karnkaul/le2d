#pragma once
#include <kvf/rect.hpp>
#include <kvf/render_target.hpp>
#include <le2d/primitive.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/shader.hpp>
#include <le2d/user_draw_data.hpp>

namespace le {
class RenderPass;
class ResourcePool;

class Renderer {
  public:
	using UserData = UserDrawData;

	Renderer(Renderer const&) = delete;
	Renderer(Renderer&&) = delete;
	auto operator=(Renderer const&) = delete;
	auto operator=(Renderer&&) = delete;

	Renderer() = default;

	explicit Renderer(RenderPass& render_pass, ResourcePool& resource_pool, vk::CommandBuffer command_buffer);
	~Renderer() { end_render(); }

	auto set_line_width(float width) -> bool;
	auto set_shader(Shader const& shader) -> bool;
	auto set_render_area(kvf::UvRect const& n_rect) -> bool;
	auto set_scissor_rect(kvf::UvRect n_rect) -> bool;
	auto set_user_data(UserDrawData const& user_data) -> bool;

	auto draw(Primitive const& primitive, std::span<RenderInstance const> instances) -> bool;

	[[nodiscard]] auto is_rendering() const -> bool { return m_pass != nullptr; }
	auto end_render() -> kvf::RenderTarget;

	[[nodiscard]] auto command_buffer() const -> vk::CommandBuffer { return m_cmd; }

	explicit operator bool() const { return is_rendering(); }

	Transform view{};
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};

  private:
	auto bind_shader(vk::PrimitiveTopology topology) -> bool;

	RenderPass* m_pass{};
	ResourcePool* m_resource_pool{};
	vk::CommandBuffer m_cmd{};

	Shader const* m_shader{};
	vk::Viewport m_viewport{};
	vk::Rect2D m_scissor{};
	UserDrawData m_user_data{};

	vk::Pipeline m_pipeline{};
	float m_line_width{1.0f};
};
} // namespace le
