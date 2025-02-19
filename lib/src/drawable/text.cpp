#include <le2d/drawable/text.hpp>

namespace le::drawable {
void TextGeometry::set_string(FontAtlas const& font_atlas, std::string_view const line, TextExpand const expand) {
	m_size = {};
	m_verts.clear();

	m_glyph_layouts.clear();
	font_atlas.push_layouts(m_glyph_layouts, line);

	auto const rect = kvf::ttf::glyph_bounds(m_glyph_layouts);
	m_size = rect.size();

	auto offset = glm::vec2{};
	switch (expand) {
	case TextExpand::eBoth: offset.x -= 0.5f * m_size.x; break;
	case TextExpand::eLeft: offset.x -= m_size.x; break;
	default: break;
	}

	m_verts.clear();
	write_glyphs(m_verts, m_glyph_layouts, offset);
}

auto TextBase::get_primitive() const -> Primitive {
	return Primitive{
		.vertices = m_geometry.get_vertices(),
		.indices = m_geometry.get_indices(),
		.topology = m_geometry.get_topology(),
		.texture = get_texture(),
	};
}

void TextBase::set_string(Font& font, std::string_view const line, Params const& params) {
	if (!font.is_loaded()) { return; }

	auto const& atlas = font.get_atlas(params.height);
	m_geometry.set_string(atlas, line, params.expand);
	m_texture = &atlas.get_texture();
}
} // namespace le::drawable
