#pragma once
#include "kvf/aspect_resize.hpp"
#include "le2d/drawable/draw_instance.hpp"
#include "le2d/shape/quad.hpp"

namespace le::drawable {
/// \brief Base class for Sprite types.
/// Sprites have a "base size" property via which the actual size is determined,
/// based on the set ResizeAspect logic and aspect ratio of UV coordinates.
class SpriteBase : public IDrawPrimitive {
  public:
	explicit SpriteBase(glm::vec2 const size = glm::vec2{200.0f}) { set_base_size(size); }

	[[nodiscard]] auto get_geometry() const -> IGeometry const& final { return m_quad; }
	[[nodiscard]] auto get_texture() const -> klib::Ptr<ITextureBase const> final { return m_texture; }

	[[nodiscard]] auto get_base_size() const -> glm::vec2 { return m_size; }
	void set_base_size(glm::vec2 size);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_quad.get_size(); }

	[[nodiscard]] auto get_origin() const -> glm::vec2 { return m_quad.get_origin(); }
	void set_origin(glm::vec2 origin);

	[[nodiscard]] auto get_uv() const -> kvf::UvRect { return m_quad.get_uv(); }
	void set_uv(kvf::UvRect const& uv);

	void set_texture(klib::Ptr<ITextureBase const> texture, kvf::UvRect const& uv = kvf::uv_rect_v);
	void set_tile(klib::Ptr<ITileSheet const> sheet, TileId tile_id);

	[[nodiscard]] auto get_resize_aspect() const -> kvf::ResizeAspect { return m_aspect; }
	void set_resize_aspect(kvf::ResizeAspect aspect);

  protected:
	void update(glm::vec2 base_size, glm::vec2 origin, kvf::UvRect const& uv);

	shape::Quad m_quad{};
	klib::Ptr<ITextureBase const> m_texture{};
	glm::vec2 m_size{};
	kvf::ResizeAspect m_aspect{kvf::ResizeAspect::None};
};

/// \brief Sprite drawable.
class Sprite : public DrawInstance<SpriteBase> {};
/// \brief Sprite drawable for multiple instances.
class InstancedSprite : public DrawInstances<SpriteBase> {};
} // namespace le::drawable
