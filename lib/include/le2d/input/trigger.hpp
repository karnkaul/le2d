#pragma once
#include "le2d/event.hpp"
#include <GLFW/glfw3.h>

namespace le::input {
class TriggerBase {
  public:
	TriggerBase() = default;

	explicit constexpr TriggerBase(int const actor) : m_actor(actor) {}

	constexpr auto on_action(int const actor, int const action) -> bool {
		if (actor != m_actor) { return false; }
		switch (action) {
		case GLFW_PRESS: m_engaged = true; return true;
		case GLFW_RELEASE: m_engaged = false; return true;
		default: return false;
		}
	}

	/// \returns true if matching key/button was pressed and not released.
	[[nodiscard]] constexpr auto is_engaged() const -> bool { return m_engaged; }
	/// \brief Force disengage even if matching key/button is currently pressed.
	constexpr void disengage() { m_engaged = false; }

  private:
	int m_actor{};
	bool m_engaged{};
};

/// \brief Keyboard key press engages a trigger.
/// Engagement is persistent (on press until release), use Chords for on-action engagement.
class KeyTrigger : public TriggerBase {
  public:
	KeyTrigger() = default;

	/// \param key Matching keyboard key (GLFW_KEY_*).
	explicit constexpr KeyTrigger(int const key) : TriggerBase(key) {}

	/// \param key Keyboard key event.
	constexpr auto on_event(event::Key const& key) -> bool { return on_action(key.key, key.action); }
};

/// \brief Mouse button press engages a trigger.
/// Engagement is persistent (on press until release), use Chords for on-action engagement.
class MouseButtonTrigger : public TriggerBase {
  public:
	MouseButtonTrigger() = default;

	/// \param button Matching mouse button (GLFW_MOUSE_BUTTON_*).
	explicit constexpr MouseButtonTrigger(int const button) : TriggerBase(button) {}

	/// \param button Mouse button event.
	constexpr auto on_event(event::MouseButton const& button) -> bool { return on_action(button.button, button.action); }
};
} // namespace le::input
