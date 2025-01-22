#include <GLFW/glfw3.h>
#include <le2d/drawables/input_text.hpp>
#include <cmath>
#include <numbers>

namespace le::drawable {
InputText::InputText(gsl::not_null<Font*> font, Params const& params)
	: m_line_input(font, params.height), m_cursor(font), m_cursor_color(params.cursor_color), m_blink_period(params.blink_period) {
	auto const text_params = TextParams{
		.height = m_line_input.get_height(),
		.expand = TextExpand::eRight,
	};
	m_cursor.set_string({&params.cursor_symbol, 1}, text_params);

	auto const rect = kvf::ttf::glyph_bounds(m_cursor.get_glyph_layouts());
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

void InputText::write(char const ch) {
	m_line_input.write(ch);
	update();
}

void InputText::backspace() {
	m_line_input.backspace();
	update();
}

void InputText::delete_front() {
	m_line_input.delete_front();
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

void InputText::on_key(event::Key const& key) {
	if (!is_interactive() || key.action == GLFW_RELEASE) { return; }

	switch (key.key) {
	case GLFW_KEY_LEFT: cursor_left(); break;
	case GLFW_KEY_RIGHT: cursor_right(); break;
	case GLFW_KEY_HOME: cursor_home(); break;
	case GLFW_KEY_END: cursor_end(); break;
	case GLFW_KEY_BACKSPACE: backspace(); break;
	case GLFW_KEY_DELETE: delete_front(); break;
	}
}

void InputText::on_codepoint(event::Codepoint const codepoint) {
	if (!is_interactive()) { return; }
	write(char(codepoint));
}

void InputText::tick(kvf::Seconds const dt) {
	if (!is_interactive()) { return; }
	m_elapsed += dt;
	auto const angle = (m_elapsed / m_blink_period) * 2.0f * std::numbers::pi_v<float>;
	m_cursor_alpha = 0.5f * (std::cos(angle) + 1.0f);
}

void InputText::draw(Renderer& renderer) const {
	auto const primitive = Primitive{
		.vertices = m_line_input.get_vertices().vertices,
		.indices = m_line_input.get_vertices().indices,
		.texture = &m_line_input.get_texture(),
	};
	renderer.draw(primitive, {&instance, 1});
	if (!is_interactive()) { return; }

	auto cursor_instance = RenderInstance{
		.transform = instance.transform,
		.tint = m_cursor_color,
	};
	cursor_instance.transform.position.x += m_line_input.get_cursor_x() + m_cursor_offset_x;
	cursor_instance.tint.w = kvf::Color::to_u8(m_cursor_alpha);
	renderer.draw(m_cursor.get_primitive(), {&cursor_instance, 1});
}

void InputText::update() {
	m_size = m_line_input.get_size();
	reset_blink();
}

void InputText::reset_blink() { m_elapsed = 0s; }
} // namespace le::drawable
