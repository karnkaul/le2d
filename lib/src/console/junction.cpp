#include "le2d/console/junction.hpp"

namespace le::console {
void Junction::dispatch(std::span<Event const> events) {
	m_event_buffer.clear();
	m_event_buffer.append_range(events);

	auto const terminal_was_active = m_terminal->is_active();
	std::erase_if(m_event_buffer, [&](Event const& event) { return m_terminal->consume_event(event); });

	if (m_terminal->is_active() && !terminal_was_active) { m_router->disengage(); }
	m_router->dispatch(m_event_buffer);
}
} // namespace le::console
