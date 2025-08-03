#include <kvf/is_positive.hpp>
#include <le2d/console/junction.hpp>

namespace le::console {
void Junction::dispatch(std::span<Event const> events, glm::ivec2 const framebuffer_size) const {
	auto terminal_activated = false;
	m_terminal->handle_events(framebuffer_size, events, &terminal_activated);
	if (terminal_activated) { m_router->disengage(); }
	if (!m_terminal->is_active()) { m_router->dispatch(events); }
}
} // namespace le::console
