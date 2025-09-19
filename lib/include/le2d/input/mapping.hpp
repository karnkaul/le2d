#pragma once
#include <klib/base_types.hpp>
#include <le2d/event.hpp>
#include <le2d/input/gamepad.hpp>
#include <span>

namespace le::input {
/// \brief Interface for a set of input bindings.
class IMapping : public klib::Polymorphic {
  public:
	/// \brief Process events and dispatch callbacks.
	/// \param events List of events for this frame.
	/// \param gamepads Gamepad manager instance.
	virtual void dispatch(std::span<le::Event const> events, Gamepad::Manager const& gamepads) = 0;

	/// \brief Disengage all bindings.
	virtual void disengage() = 0;
};
} // namespace le::input
