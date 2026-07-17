#pragma once
#include "le2d/input/action_types.hpp"
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

	auto consume_event(event::Key const& key) -> bool override;
	auto consume_event(event::MouseButton const& button) -> bool override;
	auto consume_event(event::Scroll const& scroll) -> bool override;
	auto consume_event(event::CursorPos const& pos) -> bool override;
	auto consume_gamepads(Gamepad::Manager const& gamepads) -> bool override;
	void dispatch_events() override;
	void disengage_input() override;

	auto consume_event(event::WindowClose /*event*/) -> bool override { return false; }
	auto consume_event(event::SwapchainResize const& /*event*/) -> bool override { return false; }
	auto consume_event(event::WindowResize const& /*event*/) -> bool override { return false; }
	auto consume_event(event::WindowFocus /*event*/) -> bool override { return false; }
	auto consume_event(event::CursorFocus /*event*/) -> bool override { return false; }
	auto consume_event(event::WindowIconify /*event*/) -> bool override { return false; }
	auto consume_event(event::Drop const& /*event*/) -> bool override { return false; }
	auto consume_event(event::Codepoint /*event*/) -> bool override { return false; }

  private:
	template <typename F>
	auto consume_action_any(F f) const -> bool;

	struct Binding {
		gsl::not_null<IAction*> action;
		std::vector<OnAction> callbacks{};

		void update_gamepads(Gamepad::Manager const& gamepads) const;
		void dispatch();
	};

	std::vector<Binding> m_bindings{};
};
} // namespace le::input
