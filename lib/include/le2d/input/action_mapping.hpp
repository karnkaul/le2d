#pragma once
#include "le2d/input/action.hpp"
#include "le2d/input/mapping.hpp"
#include <functional>
#include <gsl/pointers>
#include <vector>

namespace le::input {
/// \brief Action callback type.
using OnAction = std::move_only_function<void(action::Value const&)>;

/// \brief Stores bindings of actions to callbacks.
class ActionMapping : public IMapping {
  public:
	ActionMapping() = default;

	/// \brief Bind an action to a callback.
	/// \param action Persistent pointer to target action.
	/// \param on_action action callback.
	void bind_action(gsl::not_null<IAction*> action, OnAction on_action);

	/// \brief Remove existing bindings for an action, if any.
	/// \param action Action to remove bindings for.
	void unbind_action(gsl::not_null<IAction const*> action);

	/// \brief Remove all existing bindings.
	void clear_bindings() { m_bindings.clear(); }

	/// \brief Process events and dispatch callbacks.
	/// \param events List of events for this frame.
	/// \param gamepads Gamepad manager instance.
	void dispatch(std::span<le::Event const> events, Gamepad::Manager const& gamepads) override;

	/// \brief Disengage all bindings.
	void disengage() override;

  private:
	template <typename F>
	void iterate(F f) const;

	struct Binding {
		gsl::not_null<IAction*> action;
		std::vector<OnAction> callbacks{};

		void update_gamepad(Gamepad::Manager const& gamepads) const;
		void dispatch();
	};

	std::vector<Binding> m_bindings{};
};
} // namespace le::input
