#include <klib/assert.hpp>
#include <le2d/line_input.hpp>

namespace le {
LineInput::LineInput(gsl::not_null<Font*> font, TextHeight const height) : m_atlas(&font->get_atlas(height)) {}

void LineInput::set_string(std::string line) {
	if (m_line == line) { return; }
	m_line = std::move(line);
	m_cursor = int(m_line.size());
	update();
}

void LineInput::write(char const ch) {
	KLIB_ASSERT(m_cursor >= 0 && m_cursor <= int(m_line.size()));
	m_line.insert(std::size_t(m_cursor), 1, ch);
	++m_cursor;
	update();
}

void LineInput::backspace() {
	if (m_cursor == 0) { return; }
	KLIB_ASSERT(m_cursor > 0 && m_cursor <= int(m_line.size()));
	m_line.erase(std::size_t(m_cursor - 1), 1);
	--m_cursor;
	update();
}

void LineInput::delete_front() {
	if (m_cursor == int(m_line.size())) { return; }
	KLIB_ASSERT(m_cursor >= 0 && m_cursor <= int(m_line.size()));
	m_line.erase(std::size_t(m_cursor), 1);
	update();
}

void LineInput::set_cursor(int const cursor) {
	m_cursor = cursor;
	update_cursor_x();
}

void LineInput::move_cursor(int const delta) { set_cursor(get_cursor() + delta); }

void LineInput::update() {
	m_vertices.vertices.clear();
	m_vertices.indices.clear();

	if (m_line.empty()) {
		m_cursor_x = 0.0f;
		m_cursor = 0;
		m_size = {};
		return;
	}

	m_glyph_layouts.clear();
	auto const line_layout = kvf::ttf::LineLayout{
		.face = m_atlas->get_face(),
		.glyphs = m_atlas->get_glyphs(),
		.use_tofu = true,
	};
	m_next_glyph_x = line_layout.generate(m_glyph_layouts, m_line).x;
	m_size = kvf::ttf::glyph_bounds(m_glyph_layouts).size();
	write_glyphs(m_vertices, m_glyph_layouts);
	update_cursor_x();
}

void LineInput::update_cursor_x() {
	m_cursor = std::clamp(m_cursor, 0, int(m_line.size()));
	if (m_cursor == 0) {
		m_cursor_x = 0.0f;
		return;
	}
	if (m_cursor == int(m_line.size())) {
		m_cursor_x = m_next_glyph_x;
		return;
	}
	m_cursor_x = m_glyph_layouts.at(std::size_t(m_cursor)).baseline.x;
}
} // namespace le
