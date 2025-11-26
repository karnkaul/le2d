#include "le2d/input/action_mapping.hpp"
#include <klib/visitor.hpp>

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

void ActionMapping::dispatch(std::span<le::Event const> events, Gamepad::Manager const& gamepads) {
	auto const visitor = klib::SubVisitor{
		[this](le::event::Key const& key) { iterate([key](IAction& action) { action.on_key(key); }); },
		[this](le::event::MouseButton const& mb) { iterate([mb](IAction& action) { action.on_mouse_button(mb); }); },
		[this](le::event::Scroll const& scroll) { iterate([scroll](IAction& action) { action.on_scroll(scroll); }); },
		[this](le::event::CursorPos const& pos) { iterate([pos](IAction& action) { action.on_cursor_pos(pos); }); },
	};
	for (auto const& event : events) { std::visit(visitor, event); }
	for (auto& binding : m_bindings) {
		binding.update_gamepad(gamepads);
		binding.dispatch();
	}
}

void ActionMapping::disengage() {
	for (auto& binding : m_bindings) {
		binding.action->disengage();
		binding.dispatch();
	}
}

template <typename F>
void ActionMapping::iterate(F func) const {
	for (auto const& binding : m_bindings) { func(*binding.action); }
}

void ActionMapping::Binding::update_gamepad(Gamepad::Manager const& gamepads) const {
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
