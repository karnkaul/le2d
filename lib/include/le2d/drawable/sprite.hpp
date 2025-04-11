#pragma once
#include <kvf/aspect_resize.hpp>
#include <le2d/drawable/draw_primitive.hpp>
#include <le2d/shape/quad.hpp>
#include <le2d/vertex_bounds.hpp>

namespace le::drawable {
class SpriteBase : public IDrawPrimitive {
  public:
	static constexpr auto size_v{shape::Quad::size_v};

	[[nodiscard]] auto to_primitive() const -> Primitive final;

	[[nodiscard]] auto get_base_size() const -> glm::vec2 { return m_size; }
	void set_base_size(glm::vec2 size);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_quad.get_size(); }

	[[nodiscard]] auto get_origin() const -> glm::vec2 { return m_quad.get_origin(); }
	void set_origin(glm::vec2 origin);

	[[nodiscard]] auto get_uv() const -> kvf::UvRect { return m_quad.get_uv(); }
	void set_uv(kvf::UvRect const& uv);

	[[nodiscard]] auto get_texture() const -> ITexture const* { return m_texture; }
	void set_texture(ITexture const* texture, kvf::UvRect const& uv = kvf::uv_rect_v);
	void set_tile(TileSheet const* sheet, TileId tile_id);

	[[nodiscard]] auto get_resize_aspect() const -> kvf::ResizeAspect { return m_aspect; }
	void set_resize_aspect(kvf::ResizeAspect aspect);

  protected:
	void update(glm::vec2 base_size, glm::vec2 origin, kvf::UvRect const& uv);

	shape::Quad m_quad{};
	ITexture const* m_texture{};
	glm::vec2 m_size{};
	kvf::ResizeAspect m_aspect{kvf::ResizeAspect::None};
};

class Sprite : public SingleDrawPrimitive<SpriteBase> {
  public:
	[[nodiscard]] auto bounding_rect() const -> kvf::Rect<> { return vertex_bounds(to_primitive().vertices, transform.to_model()); }
};

class InstancedSprite : public InstancedDrawPrimitive<SpriteBase> {};
} // namespace le::drawable
