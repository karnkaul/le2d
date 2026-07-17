#include "le2d/input/router.hpp"

namespace le::input {
namespace {
template <typename... Ts>
constexpr auto holds_any_of(Event const& e) {
	return (... || std::holds_alternative<Ts>(e));
}
} // namespace

void Router::push_mapping(std::shared_ptr<IMapping> const& mapping) {
	if (!mapping) { return; }
	disengage();
	m_mappings.push_back(mapping);
}

void Router::pop_mapping() {
	if (m_mappings.empty()) { return; }
	m_mappings.pop_back();
}

void Router::remove_mapping(std::shared_ptr<IMapping> const& mapping) {
	if (!mapping) { return; }
	auto const pred = [&](std::weak_ptr<IMapping> const& ptr) {
		auto m = ptr.lock();
		return !m || m == mapping;
	};
	std::erase_if(m_mappings, pred);
}

auto Router::get_top_mapping() const -> std::weak_ptr<IMapping> {
	if (m_mappings.empty()) { return {}; }
	return m_mappings.back();
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

	pre_dispatch();
	if (should_disengage) {
		disengage();
	} else {
		invoke_if_top([&](IMapping& mapping) { mapping.dispatch_events(events, m_gamepad_manager); });
	}

	m_top = {};
}

void Router::disengage() {
	invoke_if_top([](IMapping& mapping) { mapping.disengage_input(); });
}

void Router::pre_dispatch() {
	std::erase_if(m_mappings, [](auto const& ptr) { return ptr.expired(); });
	if (m_mappings.empty()) {
		m_top = {};
		return;
	}
	m_top = m_mappings.back().lock();
}

template <typename F>
void Router::invoke_if_top(F func) {
	if (!m_top) { return; }
	func(*m_top);
}
} // namespace le::input
