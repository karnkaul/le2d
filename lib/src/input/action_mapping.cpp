#include "le2d/input/action_mapping.hpp"

namespace le::input {
void ActionMapping::bind_action(gsl::not_null<IAction*> action, OnAction on_action) {
	auto it = std::ranges::find_if(m_bindings, [action](Binding const& b) { return b.action == action; });
	if (it != m_bindings.end()) {
		it->callbacks.push_back(std::move(on_action));
	} else {
		auto binding = Binding{.action = action};
		binding.callbacks.push_back(std::move(on_action));
		m_bindings.push_back(std::move(binding));
	}
}

void ActionMapping::unbind_action(gsl::not_null<IAction const*> action) {
	std::erase_if(m_bindings, [action](Binding const& b) { return b.action == action; });
}

auto ActionMapping::consume_event(event::Key const& key) -> bool {
	return consume_action_any([&key](IAction& action) { return action.consume_key(key); });
}

auto ActionMapping::consume_event(event::MouseButton const& button) -> bool {
	return consume_action_any([&button](IAction& action) { return action.consume_mouse_button(button); });
}

auto ActionMapping::consume_event(event::Scroll const& scroll) -> bool {
	return consume_action_any([&scroll](IAction& action) { return action.consume_scroll(scroll); });
}

auto ActionMapping::consume_event(event::CursorPos const& pos) -> bool {
	return consume_action_any([&pos](IAction& action) { return action.consume_cursor_pos(pos); });
}

auto ActionMapping::consume_gamepads(Gamepad::Manager const& gamepads) -> bool {
	for (auto& binding : m_bindings) { binding.update_gamepads(gamepads); }
	return false;
}

void ActionMapping::dispatch_events() {
	for (auto& binding : m_bindings) { binding.dispatch(); }
}

void ActionMapping::disengage_input() {
	for (auto& binding : m_bindings) {
		binding.action->disengage();
		binding.dispatch();
	}
}

template <typename F>
auto ActionMapping::consume_action_any(F func) const -> bool {
	auto ret = false;
	for (auto const& binding : m_bindings) { ret |= func(*binding.action); }
	return ret;
}

void ActionMapping::Binding::update_gamepads(Gamepad::Manager const& gamepads) const {
	auto const binding = action->get_gamepad_binding();
	if (!binding) { return; }
	auto const& gamepad = gamepads.get(*binding);
	if (!gamepad.is_connected()) { return; }
	action->update_gamepad(gamepad);
}

void ActionMapping::Binding::dispatch() {
	if (!action->should_dispatch()) { return; }
	auto const value = action->get_value();
	for (auto& callback : callbacks) { callback(value); }
}
} // namespace le::input
