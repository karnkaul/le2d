#pragma once
#include <GLFW/glfw3.h>
#include <le2d/event.hpp>
#include <bitset>

namespace le::input {
template <typename EventT>
class EventTrigger {
  public:
	EventTrigger() = default;

	explicit constexpr EventTrigger(EventT const trigger) : m_trigger(trigger) {}

	constexpr void on_event(EventT const& event) {
		if (event == m_trigger) { m_engaged = true; }
	}

	[[nodiscard]] constexpr auto is_engaged() const -> bool { return m_engaged; }
	constexpr void disengage() { m_engaged = false; }

  private:
	EventT m_trigger{};
	bool m_engaged{};
};

class KeyTrigger : public EventTrigger<event::Key> {
  public:
	KeyTrigger() = default;

	explicit constexpr KeyTrigger(int const key, int const action = GLFW_PRESS, int const mods = 0)
		: EventTrigger(event::Key{.key = key, .action = action, .mods = mods}) {}
};

class MouseButtonTrigger : public EventTrigger<event::MouseButton> {
  public:
	explicit constexpr MouseButtonTrigger(int const button, int const action = GLFW_PRESS, int const mods = 0)
		: EventTrigger(event::MouseButton{.button = button, .action = action, .mods = mods}) {}
};

class DigitalAxis {
  public:
	DigitalAxis() = default;

	explicit constexpr DigitalAxis(int lo, int hi) : m_lo(lo), m_hi(hi) {}

	constexpr void on_signal(int const signal, int const action) {
		if (signal != m_lo && signal != m_hi) { return; }
		switch (action) {
		case GLFW_PRESS: on_press(signal); break;
		case GLFW_RELEASE: on_release(signal); break;
		default: break;
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

	constexpr void on_event(event::Key const& key) { on_signal(key.key, key.action); }
};

class MouseButtonAxis : public DigitalAxis {
  public:
	MouseButtonAxis() = default;

	explicit constexpr MouseButtonAxis(int const button_lo, int const button_hi) : DigitalAxis(button_lo, button_hi) {}

	constexpr void on_event(event::MouseButton const& button) { on_signal(button.button, button.action); }
};
} // namespace le::input
