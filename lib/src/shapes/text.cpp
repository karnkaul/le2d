#include <le2d/shapes/text.hpp>

namespace le::shape {
void Text::set_string(std::string_view const line, Params const& params) {
	m_size = {};
	m_vertices.clear();
	if (!m_font->is_loaded()) { return; }

	auto& atlas = m_font->get_atlas(params.height);
	m_texture = &atlas.get_texture();

	m_glyph_layouts.clear();
	atlas.push_layouts(m_glyph_layouts, line);

	auto const rect = kvf::ttf::glyph_bounds(m_glyph_layouts);
	m_size = rect.size();

	auto offset = glm::vec2{};
	switch (params.expand) {
	case TextExpand::eBoth: offset.x -= 0.5f * m_size.x; break;
	case TextExpand::eLeft: offset.x -= m_size.x; break;
	default: break;
	}

	m_vertices.clear();
	write_glyphs(m_vertices, m_glyph_layouts, offset);
}

auto Text::get_primitive() const -> Primitive {
	return Primitive{
		.vertices = m_vertices.vertices,
		.indices = m_vertices.indices,
		.texture = m_texture,
	};
}
} // namespace le::shape
