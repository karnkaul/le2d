#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <klib/base_types.hpp>
#include <le2d/event.hpp>
#include <le2d/gamepad.hpp>
#include <bitset>
#include <optional>

namespace le::input {
/// \brief Strong type wrapper for Gamepad ID.
/// Primary auto-selects the primary Gamepad, ie first Gamepad on which a button was pressed.
enum struct GamepadId : std::int8_t { Primary = -1 };

namespace action {
class Value;
} // namespace action

/// \brief Base class for all actions.
class IAction : public klib::Polymorphic {
  public:
	virtual void on_key(event::Key const& key) = 0;
	virtual void on_mouse_button(event::MouseButton const& mb) = 0;
	virtual void on_scroll(event::Scroll const& scroll) = 0;
	virtual void update_gamepad(Gamepad const& gamepad) = 0;
	virtual void disengage() = 0;
	[[nodiscard]] virtual auto should_dispatch() const -> bool = 0;
	[[nodiscard]] virtual auto get_gamepad_id() const -> std::optional<GamepadId> = 0;
	[[nodiscard]] virtual auto get_value() const -> action::Value = 0;
};

namespace action {
/// \brief Concept for gettable Value types.
template <typename Type>
concept ValueTypeT = std::same_as<Type, bool> || std::same_as<Type, float> || std::same_as<Type, glm::vec2>;

/// \brief Value associated with an action callback.
class Value {
  public:
	Value() = default;

	explicit(false) Value(bool const digital) : m_value(digital ? 1.0f : 0.0f, 0.0f) {}
	explicit(false) Value(float const axis_1d) : m_value(axis_1d, 0.0f) {}
	explicit(false) Value(glm::vec2 const axis_2d) : m_value(axis_2d) {}

	/// \returns Value as Type.
	template <ValueTypeT Type>
	[[nodiscard]] auto get() const -> Type {
		if constexpr (std::same_as<Type, bool>) {
			return m_value[0] != 0.0f;
		} else if constexpr (std::same_as<Type, float>) {
			return m_value[0];
		} else {
			return m_value;
		}
	}

  private:
	glm::vec2 m_value{};
};

/// \brief Alias for Count binary states.
template <std::size_t Count>
using Bits = std::bitset<Count>;

/// \brief Interface for digital actions.
/// Digital actions dispatch once when the match is pressed or released.
class IDigital : public IAction {
  public:
	IDigital() = default;

	explicit IDigital(int const match) : match(match) {}

	/// \brief Match identifier.
	int match{};

  protected:
	void on_input(int actor, int action);

  private:
	void disengage() final;
	[[nodiscard]] auto should_dispatch() const -> bool final;
	[[nodiscard]] auto get_value() const -> action::Value final { return m_state[0]; }

	Bits<1> m_state{};
	mutable bool m_changed{};
};

/// \brief Interface for binary 1D axis actions.
/// Axis actions dispatch the resultant value every time.
class IBinaryAxis1D : public IAction {
  public:
	IBinaryAxis1D() = default;

	/// \param lo Match for -1.
	/// \param hi Match for +1.
	explicit IBinaryAxis1D(int const lo, int const hi) : lo(lo), hi(hi) {}

	/// \brief Match for -1.
	int lo{};
	/// \brief Match for +1.
	int hi{};

  protected:
	void on_input(int actor, int action);

  private:
	void disengage() final;
	[[nodiscard]] auto should_dispatch() const -> bool final { return true; }
	[[nodiscard]] auto get_value() const -> action::Value final;

	Bits<2> m_state{};
};

/// \brief Interface for binary 2D axis actions.
/// Axis actions dispatch the resultant value every time.
class IBinaryAxis2D : public IAction {
  public:
	IBinaryAxis2D() = default;

	/// \param horz Match for X (-1, +1).
	/// \param vert Match for Y (-1, +1).
	explicit IBinaryAxis2D(glm::ivec2 const horz, glm::ivec2 const vert) : horz(horz), vert(vert) {}

	/// \brief Match for X (-1, +1).
	glm::ivec2 horz{};
	/// \brief Match for Y (-1, +1).
	glm::ivec2 vert{};

  protected:
	void on_input(int actor, int action);

  private:
	void disengage() final;
	[[nodiscard]] auto should_dispatch() const -> bool final { return true; }
	[[nodiscard]] auto get_value() const -> action::Value final;

	Bits<4> m_state{};
};

class IGamepadAxis : public IAction {
  public:
	static constexpr auto dead_zone_v = 0.05f;

	IGamepadAxis() = default;

	explicit IGamepadAxis(float const dead_zone) : dead_zone(dead_zone) {}

	/// \brief Values with magnitude below this are floored to 0.
	float dead_zone{dead_zone_v};

  private:
	void on_key(event::Key const& /*key*/) final {}
	void on_mouse_button(event::MouseButton const& /*mb*/) final {}
	void on_scroll(event::Scroll const& /*scroll*/) final {}

	[[nodiscard]] auto should_dispatch() const -> bool final { return true; }
};

/// \brief Class template for actions using key matches.
template <std::derived_from<IAction> BaseT>
class KeyBase : public BaseT {
  public:
	using BaseT::BaseT;

  private:
	void on_key(event::Key const& key) final { this->on_input(key.key, key.action); }
	void on_mouse_button(event::MouseButton const& /*mb*/) final {}
	void on_scroll(event::Scroll const& /*scroll*/) final {}
	void update_gamepad(Gamepad const& /*gamepad*/) final {}
	[[nodiscard]] auto get_gamepad_id() const -> std::optional<GamepadId> final { return {}; }
};

/// \brief Class template for actions using mouse button matches.
template <std::derived_from<IAction> BaseT>
class MouseButtonBase : public BaseT {
  public:
	using BaseT::BaseT;

  private:
	void on_key(event::Key const& /*mb*/) final {}
	void on_mouse_button(event::MouseButton const& mb) final { this->on_input(mb.button, mb.action); }
	void on_scroll(event::Scroll const& /*scroll*/) final {}
	void update_gamepad(Gamepad const& /*gamepad*/) final {}
	[[nodiscard]] auto get_gamepad_id() const -> std::optional<GamepadId> final { return {}; }
};

/// \brief Base class for mouse scroll actions.
class MouseScrollBase : public IAction {
  protected:
	enum class Dim : std::int8_t { One, Two };

	explicit MouseScrollBase(Dim const dim) : m_dim(dim) {}

  private:
	void on_key(event::Key const& /*mb*/) final {}
	void on_mouse_button(event::MouseButton const& /*mb*/) final {}
	void on_scroll(event::Scroll const& scroll) final;
	void update_gamepad(Gamepad const& /*gamepad*/) final {}
	void disengage() final { m_value = {}; }
	[[nodiscard]] auto should_dispatch() const -> bool final;
	[[nodiscard]] auto get_gamepad_id() const -> std::optional<GamepadId> final { return {}; }
	[[nodiscard]] auto get_value() const -> action::Value final;

	Dim m_dim;
	mutable glm::vec2 m_value{};
};
} // namespace action
} // namespace le::input
