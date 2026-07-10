#include "le2d/drawable/text.hpp"

namespace le::drawable {
void TextBase::set_string(IFont& font, std::string_view const line, Params const& params) {
	m_geometry.clear_vertices();
	m_glyph_layouts.clear();
	m_size = {};

	if (line.empty()) { return; }

	auto const& font_atlas = font.get_atlas(params.height);
	m_texture = &font_atlas.get_texture();

	font_atlas.push_layouts(m_glyph_layouts, line);

	auto const rect = kvf::ttf::glyph_bounds(m_glyph_layouts);
	m_size = rect.size();

	auto offset = glm::vec2{};
	switch (params.expand) {
	case TextExpand::eBoth: offset.x -= (0.5f * m_size.x) + rect.lt.x; break;
	case TextExpand::eLeft: offset.x -= m_size.x + rect.lt.x; break;
	default: break;
	}

	m_geometry.append_glyphs(m_glyph_layouts, offset);
}
} // namespace le::drawable
