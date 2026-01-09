#include "le2d/drawable/sprite.hpp"
#include <kvf/is_positive.hpp>

namespace le::drawable {
auto SpriteBase::to_primitive() const -> Primitive {
	return Primitive{
		.vertices = m_quad.get_vertices(),
		.indices = m_quad.get_indices(),
		.topology = m_quad.get_topology(),
		.texture = get_texture(),
	};
}

void SpriteBase::set_base_size(glm::vec2 size) {
	if (!kvf::is_positive(size)) { size = {}; }
	if (size == m_size) { return; }
	m_size = size;
	update(get_base_size(), get_origin(), get_uv());
}

void SpriteBase::set_origin(glm::vec2 const origin) {
	if (origin == get_origin()) { return; }
	update(get_base_size(), origin, get_uv());
}

void SpriteBase::set_uv(kvf::UvRect const& uv) {
	if (uv == get_uv()) { return; }
	update(get_base_size(), get_origin(), uv);
}

void SpriteBase::set_texture(ITextureBase const* texture, kvf::UvRect const& uv) {
	if (texture == m_texture && uv == get_uv()) { return; }
	m_texture = texture;
	update(get_base_size(), get_origin(), uv);
}

void SpriteBase::set_tile(ITileSheet const* sheet, TileId const tile_id) {
	auto const uv = sheet != nullptr ? sheet->get_uv(tile_id) : kvf::uv_rect_v;
	if (sheet == m_texture && uv == get_uv()) { return; }
	m_texture = sheet;
	update(get_base_size(), get_origin(), uv);
}

void SpriteBase::set_resize_aspect(kvf::ResizeAspect const aspect) {
	if (aspect == m_aspect) { return; }
	m_aspect = aspect;
	update(get_base_size(), get_origin(), get_uv());
}

void SpriteBase::update(glm::vec2 const base_size, glm::vec2 const origin, kvf::UvRect const& uv) {
	auto const size = [&] {
		if (m_texture == nullptr || m_aspect == kvf::ResizeAspect::None) { return base_size; }
		auto const n_size = uv.rb - uv.lt;
		auto const tile_size = n_size * glm::vec2{m_texture->get_size()};
		return kvf::aspect_resize(base_size, tile_size, m_aspect);
	}();
	m_quad.create(kvf::Rect<>::from_size(size, origin), uv);
}
} // namespace le::drawable
