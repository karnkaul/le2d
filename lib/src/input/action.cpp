#include "le2d/input/action.hpp"
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <array>
#include <utility>

namespace le::input::action {
namespace {
template <std::size_t Count>
constexpr void set_target_switch(Bits<Count>& state, std::array<int, Count> const& matches, int const actor, int const action) {
	auto const it = std::ranges::find(matches, actor);
	if (it == matches.end()) { return; }
	auto const index = std::distance(matches.begin(), it);
	auto target = state[std::size_t(index)];
	switch (action) {
	case GLFW_PRESS: target = true; break;
	case GLFW_RELEASE: target = false; break;
	}
}
} // namespace

void IDigital::on_input(int const actor, int const action) {
	auto const old_state = m_state;
	set_target_switch(m_state, std::array{match}, actor, action);
	m_changed = old_state != m_state;
}

void IDigital::disengage() {
	if (m_state[0]) {
		m_state = {};
		m_changed = true;
	}
}

auto IDigital::should_dispatch() const -> bool { return std::exchange(m_changed, false); }

void IBinaryAxis1D::on_input(int const actor, int const action) { set_target_switch(m_state, std::array{lo, hi}, actor, action); }

void IBinaryAxis1D::disengage() { m_state = {}; }

auto IBinaryAxis1D::get_value() const -> action::Value {
	auto ret = 0.0f;
	if (m_state[0]) { ret -= 1.0f; }
	if (m_state[1]) { ret += 1.0f; }
	return ret;
}

void IBinaryAxis2D::on_input(int const actor, int const action) { set_target_switch(m_state, std::array{horz.x, horz.y, vert.x, vert.y}, actor, action); }

void IBinaryAxis2D::disengage() { m_state = {}; }

auto IBinaryAxis2D::get_value() const -> action::Value {
	auto ret = glm::vec2{};
	if (m_state[0]) { ret.x -= 1.0f; }
	if (m_state[1]) { ret.x += 1.0f; }
	if (m_state[2]) { ret.y -= 1.0f; }
	if (m_state[3]) { ret.y += 1.0f; }
	auto const length_sq = glm::length2(ret);
	if (length_sq > 1.0f) { ret /= glm::sqrt(length_sq); }
	return ret;
}

void MouseScrollBase::on_scroll(event::Scroll const& scroll) { m_value += static_cast<glm::vec2>(scroll); }

auto MouseScrollBase::should_dispatch() const -> bool {
	switch (m_dim) {
	default:
	case Dim::One: return m_value.y != 0.0f;
	case Dim::Two: return m_value != glm::vec2{};
	}
}

auto MouseScrollBase::get_value() const -> action::Value {
	auto const ret = std::exchange(m_value, {});
	switch (m_dim) {
	default:
	case Dim::One: return ret.y;
	case Dim::Two: return ret;
	}
}

void Cursor::on_cursor_pos(event::CursorPos const& cursor_pos) {
	m_value = cursor_pos.normalized;
	m_changed = true;
}

auto Cursor::should_dispatch() const -> bool { return std::exchange(m_changed, false); }

void GamepadButton::update_gamepad(Gamepad const& gamepad) {
	auto const is_pressed = gamepad.is_pressed(match);
	on_input(match, is_pressed ? GLFW_PRESS : GLFW_RELEASE);
}

void GamepadAxis1D::update_gamepad(Gamepad const& gamepad) { m_value = gamepad.get_axis(axis, dead_zone); }

void GamepadAxis2D::update_gamepad(Gamepad const& gamepad) {
	m_value.x = gamepad.get_axis(x, dead_zone);
	m_value.y = gamepad.get_axis(y, dead_zone);
}
} // namespace le::input::action
