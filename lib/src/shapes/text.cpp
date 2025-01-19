#include <le2d/shapes/text.hpp>

namespace le::shape {
void Text::set_string(std::string_view const line, Params const& params) {
	m_size = {};
	m_vertices.vertices.clear();
	m_vertices.indices.clear();
	if (line.empty() || !m_font->is_loaded()) { return; }

	auto& atlas = m_font->get_atlas(params.height);
	m_texture = &atlas.get_texture();

	auto offset = glm::vec2{};
	auto const line_layout = kvf::ttf::LineLayout{
		.face = atlas.get_face(),
		.glyphs = atlas.get_glyphs(),
		.use_tofu = true,
	};
	m_glyph_layouts.clear();
	line_layout.generate(m_glyph_layouts, line);

	auto const rect = kvf::ttf::glyph_bounds(m_glyph_layouts);
	m_size = rect.size();
	switch (params.expand) {
	case TextExpand::eBoth: offset.x -= 0.5f * m_size.x; break;
	case TextExpand::eLeft: offset.x -= m_size.x; break;
	default: break;
	}

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
