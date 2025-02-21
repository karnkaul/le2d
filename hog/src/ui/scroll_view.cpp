#include <klib/visitor.hpp>
#include <ui/scroll_view.hpp>
#include <limits>

namespace hog::ui {
void ScrollView::set_framebuffer_size(glm::vec2 size) { m_framebuffer_size = size; }

auto ScrollView::consume_cursor_move(glm::vec2 const pos) -> bool {
	m_cursor_pos = pos;
	if (m_drag_start_y.has_value()) { return true; }
	auto ret = false;
	for (auto const& widget : m_widgets) { ret |= widget->consume_cursor_move(pos); }
	return ret;
}

auto ScrollView::consume_mouse_button(le::event::MouseButton const& button) -> bool {
	if (button.button != GLFW_MOUSE_BUTTON_1) { return false; }

	auto const was_idle = m_state == State::Idle;
	m_mb1_press.on_event(button);

	switch (button.action) {
	case GLFW_PRESS: {
		if (background.bounding_rect().contains(m_cursor_pos)) { m_drag_start_y = m_cursor_pos.y; }
		break;
	}
	case GLFW_RELEASE:
		m_state = State::Idle;
		m_drag_start_y.reset();
		break;
	}

	auto ret = false;
	if (was_idle && m_state == State::Idle) {
		for (auto const& widget : m_widgets) { ret |= widget->consume_mouse_button(button); }
	}
	return ret;
}

auto ScrollView::consume_scroll(le::event::Scroll const& scroll) -> bool {
	if (!background.bounding_rect().contains(m_cursor_pos)) { return false; }
	for (auto const& widget : m_widgets) { widget->disengage(); }
	move_widgets(-scroll_speed * scroll.y);
	return true;
}

void ScrollView::disengage_input() {
	for (auto const& widget : m_widgets) { widget->disengage(); }
	m_mb1_press.disengage();
	m_drag_start_y.reset();
	m_state = State::Idle;
}

void ScrollView::add_widget(std::unique_ptr<Widget> widget) {
	if (!widget) { return; }
	m_widgets.push_back(std::move(widget));
}

void ScrollView::clear_widgets() { m_widgets.clear(); }

void ScrollView::move_widgets(float const dy) {
	if (std::abs(dy) == 0.0f || m_widgets.empty()) { return; }

	auto const hs_bg = 0.5f * background.get_size();
	auto const y_top = background.transform.position.y + hs_bg.y;
	auto const y_bottom = background.transform.position.y - hs_bg.y;

	auto const& top_widget = *m_widgets.front();
	auto const& bottom_widget = *m_widgets.back();
	auto const top_widget_y = top_widget.get_position().y + (0.5f * top_widget.get_hitbox().size().y) + dy;
	auto const bottom_widget_y = bottom_widget.get_position().y - (0.5f * bottom_widget.get_hitbox().size().y) + dy;
	if (top_widget_y + y_pad > y_top || bottom_widget_y - y_pad < y_bottom) { return; }

	for (auto const& widget : m_widgets) {
		auto position = widget->get_position();
		position.y += dy;
		widget->set_position(position);
	}
}

void ScrollView::widget_to_center(Widget const& widget) {
	auto const dy = background.transform.position.y - widget.get_position().y;
	move_widgets(dy);
}

void ScrollView::tick(kvf::Seconds const dt) {
	if (m_mb1_press.is_engaged()) { do_scroll(m_cursor_pos); }
	for (auto const& widget : m_widgets) { widget->tick(dt); }

	m_prev_cursor_y = m_cursor_pos.y;
}

void ScrollView::draw(le::Renderer& renderer) const {
	set_scissor(renderer);
	background.draw(renderer);
	for (auto const& widget : m_widgets) { widget->draw(renderer); }
	renderer.set_scissor_rect(kvf::uv_rect_v);
}

auto ScrollView::widget_near_center() const -> Widget* {
	auto dy = std::numeric_limits<float>::max();
	Widget* ret{};
	for (auto const& widget : m_widgets) {
		auto const widget_dy = std::abs(background.transform.position.y - widget->get_position().y);
		if (widget_dy < dy) {
			dy = widget_dy;
			ret = widget.get();
		}
	}
	return ret;
}

void ScrollView::do_scroll(glm::vec2 const cursor_pos) {
	if (m_state == State::Idle) {
		if (!m_drag_start_y) { return; }
		auto const dy_since_drag_start = cursor_pos.y - *m_drag_start_y;
		if (std::abs(dy_since_drag_start) >= scroll_threshold) {
			for (auto const& widget : m_widgets) { widget->disengage(); }
			m_state = State::Scrolling;
			m_prev_cursor_y = cursor_pos.y;
		}
	}
	if (m_state == State::Scrolling) {
		auto const delta_y = cursor_pos.y - m_prev_cursor_y;
		move_widgets(delta_y);
	}
}

void ScrollView::set_scissor(le::Renderer& renderer) const {
	if (!kvf::is_positive(background.get_size()) || !kvf::is_positive(m_framebuffer_size)) { return; }
	auto rect = background.bounding_rect();
	rect.lt /= m_framebuffer_size;
	rect.rb /= m_framebuffer_size;
	rect.lt.x += 0.5f;
	rect.lt.y = -rect.lt.y + 0.5f;
	rect.rb.x += 0.5f;
	rect.rb.y = -rect.rb.y + 0.5f;
	renderer.set_scissor_rect(rect);
}
} // namespace hog::ui
