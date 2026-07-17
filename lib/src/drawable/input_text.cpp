#include "le2d/drawable/input_text.hpp"
#include "le2d/input/chord.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <numbers>

namespace le::drawable {
namespace {
[[nodiscard]] auto copy_to_clipboard(event::Key const& key) {
	static constexpr auto ctrl_c = input::KeyChord{GLFW_KEY_C, GLFW_PRESS, GLFW_MOD_CONTROL};
	static constexpr auto ctrl_insert = input::KeyChord{GLFW_KEY_INSERT, GLFW_PRESS, GLFW_MOD_CONTROL};
	return ctrl_c.is_engaged(key) || ctrl_insert.is_engaged(key);
}

[[nodiscard]] auto paste_from_clipboard(event::Key const& key) {
	static constexpr auto ctrl_v = input::KeyChord{GLFW_KEY_V, GLFW_PRESS, GLFW_MOD_CONTROL};
	static constexpr auto shift_insert = input::KeyChord{GLFW_KEY_INSERT, GLFW_PRESS, GLFW_MOD_SHIFT};
	return ctrl_v.is_engaged(key) || shift_insert.is_engaged(key);
}
} // namespace

InputText::InputText(gsl::not_null<IFont*> font, CreateInfo const& create_info)
	: m_font(font), m_line_input(font, create_info.height), m_cursor_color(create_info.cursor_color), m_blink_period(create_info.blink_period) {
	auto& atlas = m_line_input.get_atlas();

	auto layouts = std::vector<kvf::ttf::GlyphLayout>{};
	atlas.push_layouts(layouts, {&create_info.cursor_symbol, 1}, 1.5f, false);
	m_cursor.append_glyphs(layouts);

	auto const rect = kvf::ttf::glyph_bounds(layouts);
	m_cursor_offset_x = -rect.lt.x;
}

void InputText::set_interactive(bool const interactive) {
	m_interactive = interactive;
	if (is_interactive()) { reset_blink(); }
}

void InputText::set_string(std::string line) {
	m_line_input.set_string(std::move(line));
	update();
}

void InputText::append(std::string_view str) {
	m_line_input.append(str);
	update();
}

void InputText::write(char const ch) {
	m_line_input.write(ch);
	update();
}

void InputText::backward_delete() {
	m_line_input.backward_delete();
	update();
}

void InputText::forward_delete() {
	m_line_input.forward_delete();
	update();
}

void InputText::backward_word() {
	m_line_input.backward_word();
	update();
}

void InputText::forward_word() {
	m_line_input.forward_word();
	update();
}

void InputText::clear() {
	m_line_input.set_string({});
	update();
}

void InputText::set_cursor(int const cursor) {
	m_line_input.set_cursor(cursor);
	update();
}

auto InputText::consume_key(event::Key const& key) -> bool {
	if (!is_interactive() || key.action == GLFW_RELEASE) { return false; }

	auto const ctrl = key.mods == GLFW_MOD_CONTROL;

	auto ret = true;
	switch (key.key) {
	case GLFW_KEY_LEFT: ctrl ? backward_word() : cursor_left(); break;
	case GLFW_KEY_RIGHT: ctrl ? forward_word() : cursor_right(); break;
	case GLFW_KEY_HOME: cursor_home(); break;
	case GLFW_KEY_END: cursor_end(); break;
	case GLFW_KEY_BACKSPACE: backward_delete(); break;
	case GLFW_KEY_DELETE: forward_delete(); break;
	default: ret = false; break;
	}

	if (copy_to_clipboard(key)) {
		glfwSetClipboardString(nullptr, get_string().data());
		ret = true;
	} else if (paste_from_clipboard(key)) {
		append(glfwGetClipboardString(nullptr));
		ret = true;
	}

	return ret;
}

auto InputText::consume_codepoint(event::Codepoint const codepoint) -> bool {
	if (!is_interactive()) { return false; }
	write(char(codepoint));
	return true;
}

void InputText::tick(kvf::Seconds const dt) {
	if (!is_interactive()) { return; }
	m_elapsed += dt;
	auto const angle = (m_elapsed / m_blink_period) * 2.0f * std::numbers::pi_v<float>;
	m_cursor_alpha = 0.5f * (std::cos(angle) + 1.0f);
}

void InputText::draw(IRenderer& renderer) const {
	auto const line_primitive = m_line_input.to_primitive();
	renderer.draw(line_primitive, {&instance, 1});
	if (!is_interactive()) { return; }

	auto const cursor_primitive = Primitive{
		.vertices = m_cursor.get_vertices(),
		.indices = m_cursor.get_indices(),
		.topology = m_cursor.get_topology(),
		.texture = line_primitive.texture,
	};
	auto cursor_instance = RenderInstance{
		.transform = instance.transform,
		.tint = m_cursor_color,
	};
	cursor_instance.transform.position.x += m_line_input.get_cursor_x() + m_cursor_offset_x;
	cursor_instance.tint.w = kvf::Color::to_u8(m_cursor_alpha);
	renderer.draw(cursor_primitive, {&cursor_instance, 1});
}

void InputText::update() {
	m_size = m_line_input.get_size();
	reset_blink();
}

void InputText::reset_blink() { m_elapsed = 0s; }
} // namespace le::drawable
