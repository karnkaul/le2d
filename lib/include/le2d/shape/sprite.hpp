#pragma once
#include <klib/polymorphic.hpp>
#include <kvf/aspect_resize.hpp>
#include <le2d/shape/quad.hpp>

namespace le::shape {
class Sprite : public klib::Polymorphic {
  public:
	static constexpr auto size_v{Quad::rect_v.size()};

	explicit Sprite(glm::vec2 size = size_v) { set_base_size(size); }

	[[nodiscard]] auto get_base_size() const -> glm::vec2 { return m_size; }
	void set_base_size(glm::vec2 size);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_quad.get_size(); }

	[[nodiscard]] auto get_origin() const -> glm::vec2 { return m_quad.get_origin(); }
	void set_origin(glm::vec2 origin);

	[[nodiscard]] auto get_uv() const -> kvf::UvRect { return m_quad.get_uv(); }
	void set_uv(kvf::UvRect const& uv);

	[[nodiscard]] auto get_texture() const -> ITexture const* { return m_quad.texture; }
	void set_texture(ITexture const* texture, kvf::UvRect const& uv = kvf::uv_rect_v);

	[[nodiscard]] auto get_resize_aspect() const -> kvf::ResizeAspect { return m_aspect; }
	void set_resize_aspect(kvf::ResizeAspect aspect);

	[[nodiscard]] auto get_primitive() const -> Primitive { return m_quad.get_primitive(); }

  protected:
	void update(glm::vec2 base_size, glm::vec2 origin, kvf::UvRect const& uv);

	Quad m_quad{};
	glm::vec2 m_size{};
	kvf::ResizeAspect m_aspect{kvf::ResizeAspect::None};
};

static_assert(ShapeT<Sprite>);
} // namespace le::shape
