#include "le2d/console/junction.hpp"

namespace le::console {
void Junction::dispatch(std::span<Event const> events) const {
	auto const state_change = m_terminal->handle_events(events);
	if (state_change == StateChange::Activated) { m_router->disengage(); }
	if (m_terminal->is_active()) { return; }

	m_router->dispatch(events);
}
} // namespace le::console
