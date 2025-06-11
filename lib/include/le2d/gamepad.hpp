#pragma once
#include <GLFW/glfw3.h>
#include <klib/c_string.hpp>

namespace le {
/// \brief Snapshot of gamepad / game controller state.
/// Note: Use le::Gamepad::get_active() or le::Gamepad::get_by_id() every frame,
/// to get the latest snapshot.
class Gamepad {
  public:
	/// \param id GLFW_JOYSTICK_<ID>.
	/// \returns true if Gamepad ID is connected.
	[[nodiscard]] static auto is_connected(int id) -> bool;
	/// \returns First Gamepad which has a button pressed (if any).
	[[nodiscard]] static auto get_active() -> Gamepad;
	/// \param id GLFW_JOYSTICK_<ID>.
	/// \returns Updated Gamepad by its ID (if connected).
	[[nodiscard]] static auto get_by_id(int id) -> Gamepad;

	/// \returns ID of this Gamepad instance.
	[[nodiscard]] auto get_id() const -> int { return m_id; }
	/// \returns true if this Gamepad instance is connected.
	[[nodiscard]] auto is_connected() const -> bool { return is_connected(get_id()); }
	/// \returns Name of this Gamepad, as reported by GLFW.
	[[nodiscard]] auto get_name() const -> klib::CString;

	/// \returns true if any button is pressed.
	[[nodiscard]] auto any_button_pressed() const -> bool;
	/// \param button GLFW_GAMEPAD_BUTTON_<ID>.
	/// \returns true if button is pressed.
	[[nodiscard]] auto is_pressed(int button) const -> bool;
	/// \param axis GLFW_GAMEPAD_AXIS_<ID>.
	/// Sticks: [-1, +1], triggers: [0, 1]. All 0 at rest.
	/// \returns (corrected) current value of axis.
	[[nodiscard]] auto get_axis(int axis) const -> float;

  private:
	static auto set_state(Gamepad& out_pad) -> bool;

	GLFWgamepadstate m_state{};
	int m_id{-1};
};
} // namespace le
