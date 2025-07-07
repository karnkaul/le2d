#include <le2d/text/text_buffer.hpp>
#include <algorithm>

namespace le {
TextBuffer::TextBuffer(gsl::not_null<IFontAtlas*> atlas, std::size_t const limit, float n_line_spacing)
	: m_atlas(atlas), m_limit(limit), m_n_line_spacing(n_line_spacing) {}

void TextBuffer::push_front(std::span<std::string> lines, kvf::Color color) {
	for (auto& line : lines) { m_lines.push_front(Line{.text = std::move(line), .color = color}); }
	while (m_lines.size() > m_limit) { m_lines.pop_back(); }
	refresh();
}

auto TextBuffer::to_primitive() const -> Primitive { return m_geometry.to_primitive(m_atlas->get_texture()); }

void TextBuffer::refresh() {
	m_geometry.clear_vertices();
	m_size = {};

	auto pos = glm::vec2{};
	for (auto const& line : m_lines) {
		if (!line.text.empty()) {
			m_layouts.clear();
			m_atlas->push_layouts(m_layouts, line.text);
			auto const& last_glyph = m_layouts.back();
			m_size.x = std::max(m_size.x, last_glyph.baseline.x + last_glyph.glyph->size.x);
			m_geometry.append_glyphs(m_layouts, pos, line.color);
		}
		pos.x = 0.0f;
		pos.y += m_n_line_spacing * float(m_atlas->get_height());
		m_size.y = pos.y;
	}
}
} // namespace le
