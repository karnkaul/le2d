#include "le2d/text/line_input.hpp"
#include "klib/debug/assert.hpp"

namespace le {
namespace {
[[nodiscard]] constexpr auto is_space(char const c) { return c == ' ' || c == '\t'; }
} // namespace

LineInput::LineInput(gsl::not_null<IFont*> font, TextHeight const height) : m_atlas(&font->get_atlas(height)) {}

auto LineInput::to_primitive() const -> Primitive { return m_geometry.to_primitive(m_atlas->get_texture()); }

void LineInput::set_string(std::string line) {
	if (m_line == line) { return; }
	m_line = std::move(line);
	set_cursor(int(m_line.size()));
	update();
}

void LineInput::append(std::string_view const str) {
	if (str.empty()) { return; }
	m_line.append(str);
	set_cursor(int(m_line.size()));
	update();
}

void LineInput::write(char const ch) {
	KLIB_ASSERT(m_cursor >= 0 && m_cursor <= int(m_line.size()));
	m_line.insert(std::size_t(m_cursor), 1, ch);
	++m_cursor;
	update();
}

void LineInput::backward_delete() {
	if (m_cursor == 0) { return; }
	KLIB_ASSERT(m_cursor > 0 && m_cursor <= int(m_line.size()));
	m_line.erase(std::size_t(m_cursor - 1), 1);
	--m_cursor;
	update();
}

void LineInput::forward_delete() {
	if (m_cursor == int(m_line.size())) { return; }
	KLIB_ASSERT(m_cursor >= 0 && m_cursor <= int(m_line.size()));
	m_line.erase(std::size_t(m_cursor), 1);
	update();
}

void LineInput::forward_word() {
	if (m_line.empty() || m_cursor == int(m_line.size())) { return; }
	auto cursor = forward_from_if(m_cursor, [](char const c) { return !is_space(c); });
	cursor = forward_from_if(cursor, [](char const c) { return is_space(c); });
	set_cursor(cursor);
}

void LineInput::backward_word() {
	if (m_line.empty() || m_cursor == 0) { return; }
	auto cursor = backward_from_if(m_cursor, [](char const c) { return is_space(c); });
	cursor = backward_from_if(cursor, [](char const c) { return !is_space(c); });
	set_cursor(cursor);
}

void LineInput::set_cursor(int const cursor) {
	m_cursor = cursor;
	update_cursor_x();
}

void LineInput::move_cursor(int const delta) { set_cursor(get_cursor() + delta); }

void LineInput::update() {
	m_geometry.clear_vertices();

	if (m_line.empty()) {
		m_cursor_x = 0.0f;
		m_cursor = 0;
		m_size = {};
		return;
	}

	m_glyph_layouts.clear();
	m_next_glyph_x = m_atlas->push_layouts(m_glyph_layouts, m_line).x;
	m_size = kvf::ttf::glyph_bounds(m_glyph_layouts).size();
	m_geometry.append_glyphs(m_glyph_layouts);
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

template <typename PredT>
auto LineInput::forward_from_if(int cursor, PredT pred) const -> int {
	while (cursor < int(m_line.size()) && pred(m_line.at(std::size_t(cursor)))) { ++cursor; }
	return cursor;
}

template <typename PredT>
auto LineInput::backward_from_if(int cursor, PredT pred) const -> int {
	while (cursor > 0 && pred(m_line.at(std::size_t(cursor - 1)))) { --cursor; }
	return cursor;
}
} // namespace le
