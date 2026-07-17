#pragma once
#include "le2d/event.hpp"
#include <GLFW/glfw3.h>

namespace le::input {
class ChordBase {
  public:
	ChordBase() = default;

	explicit constexpr ChordBase(int const actor, int const action, int const mods) : m_actor(actor), m_action(action), m_mods(mods) {}

	[[nodiscard]] constexpr auto is_engaged(int const actor, int const action, int const mods) const -> bool {
		return m_actor == actor && m_action == action && m_mods == mods;
	}

  private:
	int m_actor{};
	int m_action{};
	int m_mods{};
};

/// \brief Combination of a keyboard key action and key modifiers engage a chord.
/// Engagement is on action only, use Triggers for persistent engagement.
class KeyChord : public ChordBase {
  public:
	KeyChord() = default;

	/// \param key Matching keyboard key (GLFW_KEY_*).
	/// \param action Matching action (GLFW_PRESS, GLFW_REPEAT, GLFW_RELEASE).
	/// \param mods Matching modifiers (GLFW_MOD_*).
	explicit constexpr KeyChord(int const key, int const action = GLFW_PRESS, int const mods = 0) : ChordBase(key, action, mods) {}

	/// \returns true if key is a match for this chord.
	[[nodiscard]] constexpr auto is_engaged(event::Key const& key) const -> bool { return ChordBase::is_engaged(key.key, key.action, key.mods); }
};

/// \brief Combination of a mouse button action and key modifiers engage a chord.
/// Engagement is on action only, use Triggers for persistent engagement.
class MouseButtonChord : public ChordBase {
  public:
	MouseButtonChord() = default;

	/// \param button Matching mouse button (GLFW_MOUSE_BUTTON_*).
	/// \param action Matching action (GLFW_PRESS, GLFW_REPEAT, GLFW_RELEASE).
	/// \param mods Matching modifiers (GLFW_MOD_*).
	explicit constexpr MouseButtonChord(int const button, int const action = GLFW_PRESS, int const mods = 0) : ChordBase(button, action, mods) {}

	/// \returns true if button is a match for this chord.
	[[nodiscard]] constexpr auto is_engaged(event::MouseButton const& button) const -> bool {
		return ChordBase::is_engaged(button.button, button.action, button.mods);
	}
};
} // namespace le::input
