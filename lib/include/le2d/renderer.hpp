#pragma once
#include <klib/base_types.hpp>
#include <kvf/rect.hpp>
#include <kvf/render_pass_fwd.hpp>
#include <kvf/render_target.hpp>
#include <le2d/primitive.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/render_stats.hpp>
#include <le2d/resource/shader.hpp>
#include <le2d/user_draw_data.hpp>

namespace le {
class IRenderer : public klib::Polymorphic {
  public:
	static constexpr auto min_size_v{32};
	static constexpr auto max_size_v{4 * 4096};

	using UserData = UserDrawData;

	[[nodiscard]] virtual auto command_buffer() const -> vk::CommandBuffer = 0;
	[[nodiscard]] virtual auto get_stats() const -> RenderStats const& = 0;

	/// \returns true if rendering has begun.
	[[nodiscard]] auto is_rendering() const -> bool { return command_buffer() != vk::CommandBuffer{}; }

	/// \brief Begin rendering.
	/// \param command_buffer Render Command Buffer.
	/// \param size Desired RenderTarget / framebuffer size.
	/// \param clear Clear color (in sRGB space).
	/// \returns false if already rendering.
	virtual auto begin_render(vk::CommandBuffer command_buffer, glm::ivec2 size, kvf::Color clear = kvf::black_v) -> bool = 0;
	/// \brief End rendering.
	virtual auto end_render() -> kvf::RenderTarget const& = 0;

	virtual void set_line_width(float width) = 0;
	virtual void set_shader(IShader const& shader) = 0;
	virtual void set_render_area(kvf::UvRect const& n_rect) = 0;
	virtual void set_scissor_rect(kvf::UvRect const& n_rect) = 0;
	virtual void set_user_data(UserDrawData const& user_data) = 0;

	/// \brief Draw given instances of a Primitive.
	/// \param primitive Primitive to draw.
	/// \param instances Render Instances to draw.
	virtual void draw(Primitive const& primitive, std::span<RenderInstance const> instances) = 0;

	explicit operator bool() const { return is_rendering(); }

	/// \brief Render view (generates view matrix).
	Transform view{};
	/// \brief Fill mode.
	vk::PolygonMode polygon_mode{vk::PolygonMode::eFill};
};
} // namespace le
