#include "le2d/console/junction.hpp"

namespace le::console {
void Junction::dispatch(std::span<Event const> events, glm::ivec2 const framebuffer_size) const {
	if (m_terminal) {
		auto const state_change = m_terminal->handle_events(framebuffer_size, events);
		if (state_change == StateChange::Activated) { m_router->disengage(); }
		if (m_terminal->is_active()) { return; }
	}
	m_router->dispatch(events);
}
} // namespace le::console
