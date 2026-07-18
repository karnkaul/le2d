#pragma once
#include "klib/base_types.hpp"
#include "le2d/event.hpp"
#include "le2d/input/gamepad.hpp"

namespace le::input {
/// \brief Interface for receiving input events.
/// Consuming an event disengages the rest of the mappings lower down the stack,
/// which do not receive that event either.
/// Gamepads should generally not be consumed, except by eg GUI modals.
class IMapping : public klib::Polymorphic {
  public:
	virtual auto consume_event(event::WindowClose event) -> bool = 0;
	virtual auto consume_event(event::SwapchainResize const& event) -> bool = 0;
	virtual auto consume_event(event::WindowResize const& event) -> bool = 0;
	virtual auto consume_event(event::WindowFocus event) -> bool = 0;
	virtual auto consume_event(event::CursorFocus event) -> bool = 0;
	virtual auto consume_event(event::WindowIconify event) -> bool = 0;
	virtual auto consume_event(event::Drop const& event) -> bool = 0;
	virtual auto consume_event(event::CursorPos const& event) -> bool = 0;

	virtual auto consume_event(event::Codepoint event) -> bool = 0;
	virtual auto consume_event(event::Key const& event) -> bool = 0;
	virtual auto consume_event(event::MouseButton const& event) -> bool = 0;
	virtual auto consume_event(event::Scroll const& event) -> bool = 0;

	virtual auto consume_gamepads(Gamepad::Manager const& gamepads) -> bool = 0;

	/// \brief Events finalized, execute dispatch (if relevant).
	virtual void dispatch_events() = 0;

	/// \brief Disengage and 0-reset any cached/computed input.
	virtual void disengage_input() = 0;
};
} // namespace le::input
