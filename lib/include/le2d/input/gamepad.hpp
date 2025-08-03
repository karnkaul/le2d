#pragma once
#include <GLFW/glfw3.h>
#include <klib/c_string.hpp>
#include <array>
#include <optional>
#include <variant>

namespace le::input {
inline constexpr std::size_t max_gamepads_v = GLFW_JOYSTICK_16;

/// \brief Snapshot of gamepad / game controller state.
/// Note: Use le::Gamepad::get_active() or le::Gamepad::get_by_id() every frame,
/// to get the latest snapshot.
class Gamepad {
  public:
	static constexpr float dead_zone_v = 0.08f;
	static constexpr float nonzero_dead_zone_v = 0.2f;

	class Manager;

	/// \brief Strong type wrapper for Gamepad ID.
	enum struct Id : std::int8_t {};
	/// \brief Selector for first used gamepad.
	struct FirstUsed {};
	/// \brief Selector for last used gamepad.
	struct LastUsed {};
	/// \brief Variant of selectors.
	using Binding = std::variant<FirstUsed, LastUsed, Id>;

	/// \returns ID of this Gamepad instance.
	[[nodiscard]] auto get_id() const -> Id { return m_id; }
	/// \returns true if this Gamepad instance is connected.
	[[nodiscard]] auto is_connected() const -> bool { return m_connected; }
	/// \returns Name of this Gamepad, as reported by GLFW.
	[[nodiscard]] auto get_name() const -> klib::CString;

	/// \returns true if any button is pressed.
	[[nodiscard]] auto any_button_pressed() const -> bool;
	/// \param dead_zone Magnitude floor.
	/// \returns true if any axis value is above dead_zone.
	[[nodiscard]] auto any_axis_nonzero(float dead_zone = nonzero_dead_zone_v) const -> bool;
	/// \param button GLFW_GAMEPAD_BUTTON_<ID>.
	/// \returns true if button is pressed.
	[[nodiscard]] auto is_pressed(int button) const -> bool;
	/// \param axis GLFW_GAMEPAD_AXIS_<ID>.
	/// \param dead_zone Magnitude floor.
	/// \returns (corrected) current value of axis.
	/// Sticks: [-1, +1], triggers: [0, 1]. All 0 at rest.
	[[nodiscard]] auto get_axis(int axis, float dead_zone = dead_zone_v) const -> float;

  private:
	GLFWgamepadstate m_state{};
	Id m_id{-1};
	bool m_connected{};
};

/// \brief Stores and maintains Gamepad states.
class Gamepad::Manager {
  public:
	explicit Manager();

	[[nodiscard]] auto get_by_id(Id id) const -> Gamepad const&;
	[[nodiscard]] auto get_first_used() const -> Gamepad const&;
	[[nodiscard]] auto get_last_used() const -> Gamepad const&;
	[[nodiscard]] auto get(Binding binding) const -> Gamepad const&;

	/// \brief Update all Gamepad states.
	/// \param nonzero_dead_zone Dead zone for setting first/last used Gamepad.
	/// \returns true if a Gamepad button was pressed.
	auto update(float nonzero_dead_zone = nonzero_dead_zone_v) -> bool;

  private:
	std::array<Gamepad, max_gamepads_v> m_gamepads{};
	std::optional<Gamepad::Id> m_first_used{};
	std::optional<Gamepad::Id> m_last_used{};
};
} // namespace le::input
