#pragma once
#include <le2d/input/action_types.hpp>

namespace le::input::action {
/// \brief Digital action for a key match.
class KeyDigital : public KeyBase<IDigital> {
  public:
	KeyDigital() = default;

	/// \param trigger Matching key.
	explicit KeyDigital(int const trigger) : KeyBase<IDigital>(trigger) {}
};

/// \brief 1D axis action for key matches.
class KeyAxis1D : public KeyBase<IBinaryAxis1D> {
  public:
	KeyAxis1D() = default;

	/// \param lo Matching keys for -1.
	/// \param hi Matching keys for +1.
	explicit KeyAxis1D(int const lo, int const hi) : KeyBase<IBinaryAxis1D>(lo, hi) {}
};

/// \brief 2D axis action for key matches.
class KeyAxis2D : public KeyBase<IBinaryAxis2D> {
  public:
	KeyAxis2D() = default;

	/// \param horz Matching keys for X (-1, +1).
	/// \param vert Matching keys for Y (-1, +1).
	explicit KeyAxis2D(glm::ivec2 const horz, glm::ivec2 const vert) : KeyBase<IBinaryAxis2D>(horz, vert) {}
};

/// \brief Digital action for a mouse button match.
class MouseButtonDigital : public MouseButtonBase<IDigital> {
  public:
	MouseButtonDigital() = default;

	/// \param trigger Matching mouse button.
	explicit MouseButtonDigital(int const trigger) : MouseButtonBase<IDigital>(trigger) {}
};

/// \brief 1D axis action for mouse button matches.
class MouseButtonAxis1D : public MouseButtonBase<IBinaryAxis1D> {
  public:
	MouseButtonAxis1D() = default;

	/// \param lo Matching mouse button for X (-1, +1).
	/// \param hi Matching mouse button for Y (-1, +1).
	explicit MouseButtonAxis1D(int const lo, int const hi) : MouseButtonBase<IBinaryAxis1D>(lo, hi) {}
};

/// \brief 1D Mouse scroll (Y axis).
class MouseScroll1D : public MouseScrollBase {
  public:
	explicit MouseScroll1D() : MouseScrollBase(Dim::One) {}
};

/// \brief 2D Mouse scroll (X and Y axes).
class MouseScroll2D : public MouseScrollBase {
  public:
	explicit MouseScroll2D() : MouseScrollBase(Dim::Two) {}
};

/// \brief Mouse cursor.
class Cursor : public IAction {
	void on_key(event::Key const& /*mb*/) final {}
	void on_mouse_button(event::MouseButton const& /*mb*/) final {}
	void on_scroll(event::Scroll const& /*scroll*/) final {}
	void on_cursor_pos(event::CursorPos const& cursor_pos) final;
	void update_gamepad(Gamepad const& /*gamepad*/) final {}
	void disengage() final {}
	[[nodiscard]] auto should_dispatch() const -> bool final;
	[[nodiscard]] auto get_gamepad_binding() const -> std::optional<Gamepad::Binding> final { return {}; }
	[[nodiscard]] auto get_value() const -> action::Value final { return m_value; }

	glm::vec2 m_value{};
	mutable bool m_changed{};
};

/// \brief Digital action for a primary gamepad button match.
class GamepadButton : public IDigital {
  public:
	GamepadButton() = default;

	explicit GamepadButton(int const trigger) : IDigital(trigger) {}

	/// \brief Gamepad binding.
	Gamepad::Binding binding{Gamepad::FirstUsed{}};

  private:
	void on_key(event::Key const& /*mb*/) final {}
	void on_mouse_button(event::MouseButton const& /*mb*/) final {}
	void on_scroll(event::Scroll const& /*scroll*/) final {}
	void on_cursor_pos(event::CursorPos const& /*cursor_pos*/) final {}
	void update_gamepad(Gamepad const& gamepad) final;
	[[nodiscard]] auto get_gamepad_binding() const -> std::optional<Gamepad::Binding> final { return binding; }
};

/// \brief 1D gamepad axis action.
/// Trigger (LT, RT) value range: [0, 1], axis value range: [-1, 1].
class GamepadAxis1D : public IGamepadAxis {
  public:
	GamepadAxis1D() = default;

	/// \param axis Matching axis.
	/// \param dead_zone Dead zone: values with magnitude below this are floored to 0.
	explicit GamepadAxis1D(int const axis, float const dead_zone = dead_zone_v) : IGamepadAxis(dead_zone), axis(axis) {}

	/// \brief Matching axis.
	int axis{};

  private:
	void update_gamepad(Gamepad const& gamepad) final;
	void disengage() final { m_value = {}; }
	[[nodiscard]] auto get_value() const -> action::Value final { return m_value; }

	float m_value{};
};

/// \brief 2D gamepad axis action.
class GamepadAxis2D : public IGamepadAxis {
  public:
	GamepadAxis2D() = default;

	/// \param x Matching x axis.
	/// \param y Matching y axis.
	/// \param dead_zone Dead zone: values with magnitude below this are floored to 0.
	explicit GamepadAxis2D(int const x, int const y, float const dead_zone = dead_zone_v) : IGamepadAxis(dead_zone), x(x), y(y) {}

	/// \brief Matching x axis.
	int x{};
	/// \brief Matching y axis.
	int y{};

  private:
	void update_gamepad(Gamepad const& gamepad) final;
	void disengage() final { m_value = {}; }
	[[nodiscard]] auto get_value() const -> action::Value final { return m_value; }

	glm::vec2 m_value{};
};
} // namespace le::input::action
