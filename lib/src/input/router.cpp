#include <le2d/input/router.hpp>
#include <algorithm>

namespace le::input {
void Router::push_mapping(gsl::not_null<IMapping*> mapping) {
	disengage();
	m_mappings.push_back(mapping);
}

void Router::pop_mapping() {
	if (m_mappings.empty()) { return; }
	m_mappings.pop_back();
}

void Router::dispatch(std::span<le::Event const> events) {
	if (m_gamepad_manager.update()) {
		m_last_used_device = Device::Gamepad;
	} else if (std::ranges::any_of(events, [](Event const& e) { return std::holds_alternative<event::Key>(e); })) {
		m_last_used_device = Device::Keyboard;
	} else if (std::ranges::any_of(events, [](Event const& e) { return std::holds_alternative<event::MouseButton>(e); })) {
		m_last_used_device = Device::Mouse;
	}
	do_dispatch([&](IMapping& mapping) { mapping.dispatch(events, m_gamepad_manager); });
}

void Router::disengage() {
	do_dispatch([](IMapping& mapping) { mapping.disengage(); });
}

template <typename F>
void Router::do_dispatch(F func) {
	if (m_mappings.empty()) { return; }
	auto top = m_mappings.back();
	func(*top);
}
} // namespace le::input
