#include "le2d/input/router.hpp"

namespace le::input {
namespace {
template <typename... Ts>
constexpr auto holds_any_of(Event const& e) {
	return (... || std::holds_alternative<Ts>(e));
}
} // namespace

void Router::push_mapping(gsl::not_null<IMapping*> mapping) {
	disengage();
	m_mappings.push_back(mapping);
}

void Router::pop_mapping() {
	if (m_mappings.empty()) { return; }
	m_mappings.pop_back();
}

void Router::remove_mapping(gsl::not_null<IMapping const*> mapping) {
	std::erase_if(m_mappings, [mapping](gsl::not_null<IMapping*> m) { return m == mapping; });
}

void Router::dispatch(std::span<Event const> events) {
	auto should_disengage = false;
	for (auto const& event : events) {
		if (std::holds_alternative<event::Key>(event)) {
			m_last_used_device = Device::Keyboard;
		} else if (std::holds_alternative<event::MouseButton>(event)) {
			m_last_used_device = Device::Mouse;
		} else if (!should_disengage && holds_any_of<event::WindowFocus, event::WindowResize>(event)) {
			should_disengage = true;
		}
	}

	if (m_gamepad_manager.update(nonzero_dead_zone)) { m_last_used_device = Device::Gamepad; }

	if (should_disengage) {
		disengage();
	} else {
		do_dispatch([&](IMapping& mapping) { mapping.dispatch(events, m_gamepad_manager); });
	}
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
