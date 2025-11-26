#include "le2d/console/junction.hpp"
#include <kvf/is_positive.hpp>

namespace le::console {
void Junction::dispatch(std::span<Event const> events, glm::ivec2 const framebuffer_size) const {
	auto const state_change = m_terminal->handle_events(framebuffer_size, events);
	if (state_change == StateChange::Activated) { m_router->disengage(); }
	if (!m_terminal->is_active()) { m_router->dispatch(events); }
}
} // namespace le::console
