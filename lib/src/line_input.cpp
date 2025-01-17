#include <klib/assert.hpp>
#include <le2d/line_geometry.hpp>
#include <le2d/line_input.hpp>

namespace le {
LineInput::LineInput(gsl::not_null<Font*> font, TextHeight const height) : m_atlas(&font->get_atlas(height)) {}

void LineInput::set_string(std::string line) {
	if (m_line == line) { return; }
	m_line = std::move(line);
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
	auto const new_idx = std::clamp(int(cursor), 0, int(m_line.size()));
	if (new_idx == m_cursor) { return; }
	m_cursor = new_idx;
	if (m_cursor == 0) {
		m_cursor_x = 0.0f;
	} else {
		update_cursor_x();
	}
}

void LineInput::move_cursor(int const delta) { set_cursor(get_cursor() + delta); }

void LineInput::update() {
	m_vertices.vertices.clear();
	m_vertices.indices.clear();

	if (m_line.empty()) {
		m_cursor_x = 0.0f;
		m_cursor = 0;
		return;
	}

	m_cursor = std::clamp(m_cursor, 0, int(m_line.size()));
	auto writer = LineGeometry{.glyphs = m_atlas->get_glyphs()};

	auto const left = std::string_view{m_line}.substr(0, std::size_t(m_cursor));
	if (!left.empty()) { writer.write_line(m_vertices, left); }
	m_cursor_x = writer.position.x;
	if (m_cursor == int(m_line.size())) { return; }

	auto const right = std::string_view{m_line}.substr(std::size_t(m_cursor));
	writer.write_line(m_vertices, right);
}

void LineInput::update_cursor_x() {
	auto const left = std::string_view{m_line}.substr(0, std::size_t(m_cursor));
	if (left.empty()) { m_cursor_x = 0.0f; }
	auto const writer = LineGeometry{.glyphs = m_atlas->get_glyphs()};
	m_cursor_x = writer.next_glyph_position(left).x;
}
} // namespace le
