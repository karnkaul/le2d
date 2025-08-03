#include <klib/assert.hpp>
#include <klib/visitor.hpp>
#include <le2d/input/gamepad.hpp>
#include <ranges>

namespace le::input {
namespace {
auto const g_default = Gamepad{};
} // namespace

auto Gamepad::get_name() const -> klib::CString {
	if (!is_connected()) { return {}; }
	return glfwGetGamepadName(int(m_id));
}

auto Gamepad::any_button_pressed() const -> bool {
	for (int button = GLFW_GAMEPAD_BUTTON_A; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button) {
		if (is_pressed(button)) { return true; }
	}
	return false;
}

auto Gamepad::any_axis_nonzero(float const dead_zone) const -> bool {
	return std::ranges::any_of(m_state.axes, [dead_zone](float const f) { return std::abs(f) >= dead_zone; });
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

Gamepad::Manager::Manager() {
	for (auto [id, gamepad] : std::views::enumerate(m_gamepads)) { gamepad.m_id = Id(id); }
}

auto Gamepad::Manager::get_by_id(Id const id) const -> Gamepad const& {
	auto const id_ = int(id);
	if (id_ < GLFW_JOYSTICK_1 || id_ > GLFW_JOYSTICK_LAST) { return g_default; }
	auto const& ret = m_gamepads.at(std::size_t(id_));
	if (!ret.m_connected) { return g_default; }
	return ret;
}

auto Gamepad::Manager::get_first_used() const -> Gamepad const& {
	if (!m_first_used) { return g_default; }
	auto const& ret = m_gamepads.at(std::size_t(*m_first_used));
	if (!ret.m_connected) { return g_default; }
	return ret;
}

auto Gamepad::Manager::get_last_used() const -> Gamepad const& {
	if (!m_last_used) { return g_default; }
	auto const& ret = m_gamepads.at(std::size_t(*m_last_used));
	if (!ret.m_connected) { return g_default; }
	return ret;
}

auto Gamepad::Manager::get(Binding const binding) const -> Gamepad const& {
	auto const visitor = klib::Visitor{
		[this](FirstUsed) -> Gamepad const& { return get_first_used(); },
		[this](LastUsed) -> Gamepad const& { return get_last_used(); },
		[this](Id const id) -> Gamepad const& { return get_by_id(id); },
	};
	return std::visit(visitor, binding);
}

auto Gamepad::Manager::update(float const nonzero_dead_zone) -> bool {
	static auto const fix_axis_y = [](float& in) { in = -in; };
	static auto const fix_trigger = [](float& in) { in = (1.0f + in) * 0.5f; };
	auto ret = false;
	for (auto& gamepad : m_gamepads) {
		auto const id_ = int(gamepad.m_id);
		gamepad.m_connected = glfwGetGamepadState(id_, &gamepad.m_state) == GLFW_TRUE;
		if (!gamepad.m_connected) {
			if (m_first_used == gamepad.m_id) { m_first_used.reset(); }
			if (m_last_used == gamepad.m_id) { m_last_used.reset(); }
			continue;
		}
		fix_axis_y(gamepad.m_state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
		fix_axis_y(gamepad.m_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
		fix_trigger(gamepad.m_state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
		fix_trigger(gamepad.m_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);
		if (gamepad.any_button_pressed() || gamepad.any_axis_nonzero(nonzero_dead_zone)) {
			m_last_used = gamepad.m_id;
			if (!m_first_used) { m_first_used = m_last_used; }
			ret = true;
		}
	}
	return ret;
}
} // namespace le::input
