#include <GLFW/glfw3.h>
#include <klib/visitor.hpp>
#include <ui/widget.hpp>

namespace hog::ui {
auto Widget::consume_cursor_move(glm::vec2 const pos) -> bool {
	m_cursor_pos = pos;
	if (m_state == State::Disabled) { return false; }

	if (is_cursor_hit()) {
		if (m_state == State::None) { m_state = State::Hover; }
	} else {
		m_state = State::None;
	}
	return false;
}

auto Widget::consume_mouse_button(le::event::MouseButton const& button) -> bool {
	if (button.button != GLFW_MOUSE_BUTTON_1 || m_state == State::Disabled) { return false; }

	auto const is_hit = is_cursor_hit();
	if (!is_hit) {
		m_state = State::None;
		return false;
	}

	switch (button.action) {
	case GLFW_PRESS: m_state = State::Press; return true;
	case GLFW_RELEASE: {
		if (m_state == State::Press && m_debounce_remain <= 0s) {
			on_click();
			m_debounce_remain = click_debounce;
		}
		m_state = State::Hover;
		break;
	}
	default: break;
	}

	return false;
}

void Widget::set_disabled(bool const disabled) { m_state = disabled ? State::Disabled : State::None; }

void Widget::tick(kvf::Seconds const dt) {
	if (m_debounce_remain > 0s) { m_debounce_remain -= dt; }
}

void Widget::disengage() { m_state = State::None; }

void Widget::on_cursor(le::event::CursorPos const& cursor_pos) {
	m_cursor_pos = cursor_pos.normalized;
	if (m_state == State::Disabled) { return; }

	if (is_cursor_hit()) {
		if (m_state == State::None) { m_state = State::Hover; }
	} else {
		m_state = State::None;
	}
}

auto Widget::on_button(le::event::MouseButton const& mouse_button) -> bool {
	if (mouse_button.button != GLFW_MOUSE_BUTTON_1) { return false; }

	if (m_state == State::Disabled) { return false; }

	auto const is_hit = is_cursor_hit();
	if (!is_hit) {
		m_state = State::None;
		return false;
	}

	switch (mouse_button.action) {
	case GLFW_PRESS: m_state = State::Press; return true;
	case GLFW_RELEASE: {
		if (m_debounce_remain <= 0s) {
			on_click();
			m_debounce_remain = click_debounce;
		}
		m_state = State::Hover;
		break;
	}
	default: break;
	}

	return false;
}

auto Widget::is_cursor_hit() const -> bool { return get_hitbox().contains(m_cursor_pos); }
} // namespace hog::ui
