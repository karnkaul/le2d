#pragma once
#include <GLFW/glfw3.h>
#include <le2d/event.hpp>
#include <bitset>

namespace le::input {
class Chord {
  public:
	struct Entry {
		int actor{};
		int action{};
		int mods{};

		auto operator==(Entry const&) const -> bool = default;
	};

	Chord() = default;

	explicit constexpr Chord(Entry const& match) : m_match(match) {}

	[[nodiscard]] constexpr auto is_engaged(Entry const& entry) const -> bool { return entry == m_match; }

  private:
	Entry m_match{};
};

class KeyChord : public Chord {
  public:
	KeyChord() = default;

	explicit constexpr KeyChord(int const key, int const action = GLFW_PRESS, int const mods = 0)
		: Chord(Entry{.actor = key, .action = action, .mods = mods}) {}

	[[nodiscard]] constexpr auto is_engaged(event::Key const& key) const -> bool {
		return Chord::is_engaged(Entry{.actor = key.key, .action = key.action, .mods = key.mods});
	}
};

class MouseButtonChord : public Chord {
  public:
	MouseButtonChord() = default;

	explicit constexpr MouseButtonChord(int const button, int const action = GLFW_PRESS, int const mods = 0)
		: Chord(Entry{.actor = button, .action = action, .mods = mods}) {}

	[[nodiscard]] constexpr auto is_engaged(event::MouseButton const& button) const -> bool {
		return Chord::is_engaged(Entry{.actor = button.button, .action = button.action, .mods = button.mods});
	}
};

class Trigger {
  public:
	Trigger() = default;

	explicit constexpr Trigger(int const actor) : m_actor(actor) {}

	constexpr auto on_action(int const actor, int const action) -> bool {
		if (actor != m_actor) { return false; }
		switch (action) {
		case GLFW_PRESS: m_engaged = true; return true;
		case GLFW_RELEASE: m_engaged = false; return true;
		default: return false;
		}
	}

	[[nodiscard]] constexpr auto is_engaged() const -> bool { return m_engaged; }
	constexpr void disengage() { m_engaged = false; }

  private:
	int m_actor{};
	bool m_engaged{};
};

class KeyTrigger : public Trigger {
  public:
	KeyTrigger() = default;

	explicit constexpr KeyTrigger(int const key) : Trigger(key) {}

	constexpr auto on_event(event::Key const& key) -> bool { return on_action(key.key, key.action); }
};

class MouseButtonTrigger : public Trigger {
  public:
	MouseButtonTrigger() = default;

	explicit constexpr MouseButtonTrigger(int const key) : Trigger(key) {}

	constexpr auto on_event(event::MouseButton const& button) -> bool { return on_action(button.button, button.action); }
};

class DigitalAxis {
  public:
	DigitalAxis() = default;

	explicit constexpr DigitalAxis(int lo, int hi) : m_lo(lo), m_hi(hi) {}

	constexpr auto on_action(int const actor, int const action) -> bool {
		if (actor != m_lo && actor != m_hi) { return false; }
		switch (action) {
		case GLFW_PRESS: on_press(actor); return true;
		case GLFW_RELEASE: on_release(actor); return true;
		default: return false;
		}
	}

	[[nodiscard]] constexpr auto value() const -> float {
		auto ret = 0.0f;
		if (m_held.test(0)) { ret -= 1.0f; }
		if (m_held.test(1)) { ret += 1.0f; }
		return ret;
	}

	constexpr void disengage() { m_held.reset(); }

  private:
	constexpr void on_press(int const signal) {
		if (signal == m_lo) {
			m_held.set(0);
		} else if (signal == m_hi) {
			m_held.set(1);
		}
	}

	constexpr void on_release(int const signal) {
		if (signal == m_lo) {
			m_held.reset(0);
		} else if (signal == m_hi) {
			m_held.reset(1);
		}
	}

	int m_lo{};
	int m_hi{};

	std::bitset<2> m_held{};
};

class KeyAxis : public DigitalAxis {
  public:
	KeyAxis() = default;

	explicit constexpr KeyAxis(int const key_lo, int const key_hi) : DigitalAxis(key_lo, key_hi) {}

	constexpr auto on_event(event::Key const& key) -> bool { return on_action(key.key, key.action); }
};

class MouseButtonAxis : public DigitalAxis {
  public:
	MouseButtonAxis() = default;

	explicit constexpr MouseButtonAxis(int const button_lo, int const button_hi) : DigitalAxis(button_lo, button_hi) {}

	constexpr auto on_event(event::MouseButton const& button) -> bool { return on_action(button.button, button.action); }
};
} // namespace le::input
