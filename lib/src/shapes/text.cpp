#include <le2d/line_geometry.hpp>
#include <le2d/shapes/text.hpp>

namespace le::shape {
void Text::set_string(std::string_view const line, Params const& params) {
	m_size = {};
	m_vertices.vertices.clear();
	m_vertices.indices.clear();
	if (line.empty() || !m_font->is_loaded()) { return; }

	auto& atlas = m_font->get_atlas(params.height);
	auto line_geometry = LineGeometry{.glyphs = atlas.get_glyphs()};
	m_texture = &atlas.get_texture();

	auto const rect = line_geometry.line_bounds(line);
	m_size = rect.size();
	switch (params.expand) {
	case TextExpand::eBoth: line_geometry.position.x -= 0.5f * m_size.x; break;
	case TextExpand::eLeft: line_geometry.position.x -= m_size.x; break;
	default: break;
	}

	line_geometry.write_line(m_vertices, line);
}

auto Text::get_primitive() const -> Primitive {
	return Primitive{
		.vertices = m_vertices.vertices,
		.indices = m_vertices.indices,
		.texture = m_texture,
	};
}
} // namespace le::shape
