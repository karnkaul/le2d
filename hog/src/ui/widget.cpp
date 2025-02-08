#include <GLFW/glfw3.h>
#include <ui/widget.hpp>

namespace hog::ui {
void Widget::set_framebuffer_size(glm::vec2 const framebuffer_size) { m_framebuffer_size = framebuffer_size; }

void Widget::on_cursor(le::event::CursorPos const& cursor_pos) {
	m_cursor_pos = cursor_pos.normalized;
	if (is_cursor_hit()) {
		if (m_state == State::None) { m_state = State::Hover; }
	} else {
		m_state = State::None;
	}
}

void Widget::on_button(le::event::MouseButton const& mouse_button) {
	if (mouse_button.button != GLFW_MOUSE_BUTTON_1) { return; }
	switch (mouse_button.action) {
	case GLFW_PRESS: on_mb1_press(); break;
	case GLFW_RELEASE: on_mb1_release(); break;
	default: break;
	}
}

void Widget::tick(kvf::Seconds const dt) {
	if (m_debounce_remain > 0s) { m_debounce_remain -= dt; }
}

auto Widget::is_cursor_hit() const -> bool { return get_hitbox().contains(m_cursor_pos.to_target(m_framebuffer_size)); }

void Widget::on_mb1_press() {
	auto const is_hit = is_cursor_hit();
	if (!is_hit) {
		m_state = State::None;
		return;
	}

	m_state = State::Press;
}

void Widget::on_mb1_release() {
	auto const is_hit = is_cursor_hit();
	if (!is_hit) {
		m_state = State::None;
		return;
	}

	if (m_debounce_remain <= 0s) {
		on_click();
		m_debounce_remain = click_debounce;
	}
	m_state = State::Hover;
}
} // namespace hog::ui
