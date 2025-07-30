#include <klib/assert.hpp>
#include <le2d/gamepad.hpp>

namespace le {
auto Gamepad::is_connected(int const id) -> bool { return id >= GLFW_JOYSTICK_1 && id <= GLFW_JOYSTICK_LAST && glfwJoystickIsGamepad(id) == GLFW_TRUE; }

auto Gamepad::get_active() -> Gamepad {
	for (auto id = GLFW_JOYSTICK_1; id <= GLFW_JOYSTICK_LAST; ++id) {
		auto ret = Gamepad{};
		ret.m_id = id;
		if (set_state(ret) && ret.any_button_pressed()) { return ret; }
	}

	return {};
}

auto Gamepad::get_by_id(int const id) -> Gamepad {
	auto ret = Gamepad{};
	ret.m_id = id;
	if (!set_state(ret)) { return {}; }
	return ret;
}

auto Gamepad::get_name() const -> klib::CString {
	if (!is_connected()) { return {}; }
	return glfwGetGamepadName(m_id);
}

auto Gamepad::update() -> bool {
	*this = get_by_id(get_id());
	return is_connected();
}

auto Gamepad::update_or_autoselect() -> bool {
	auto ret = is_connected();
	if (ret) {
		*this = get_by_id(get_id());
	} else {
		*this = get_active();
		ret = is_connected();
	}
	return ret;
}

auto Gamepad::any_button_pressed() const -> bool {
	for (int button = GLFW_GAMEPAD_BUTTON_A; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button) {
		if (is_pressed(button)) { return true; }
	}
	return false;
}

auto Gamepad::is_pressed(int const button) const -> bool {
	KLIB_ASSERT(button >= 0 && button < int(std::size(m_state.buttons)));
	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
	return m_state.buttons[button];
}

auto Gamepad::get_axis(int const axis, float const dead_zone) const -> float {
	KLIB_ASSERT(axis >= 0 && axis < int(std::size(m_state.axes)));
	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
	auto const ret = m_state.axes[axis];
	return std::abs(ret) < dead_zone ? 0.0f : ret;
}

auto Gamepad::set_state(Gamepad& out_pad) -> bool {
	if (!out_pad.is_connected()) { return false; }
	if (glfwGetGamepadState(out_pad.m_id, &out_pad.m_state) != GLFW_TRUE) { return false; }
	static auto const fix_axis_y = [](float& in) { in = -in; };
	static auto const fix_trigger = [](float& in) { in = (1.0f + in) * 0.5f; };
	fix_axis_y(out_pad.m_state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
	fix_axis_y(out_pad.m_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
	fix_trigger(out_pad.m_state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
	fix_trigger(out_pad.m_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);
	return true;
}
} // namespace le
