#include <le2d/event/dispatch.hpp>
#include <ranges>

namespace le::event {
Dispatch::Dispatch(Dispatch&& rhs) noexcept : m_listeners(std::move(rhs.m_listeners)) { update_listeners(this); }

auto Dispatch::operator=(Dispatch&& rhs) noexcept -> Dispatch& {
	if (&rhs != this) {
		update_listeners(nullptr);
		m_listeners = std::move(rhs.m_listeners);
		update_listeners(this);
	}
	return *this;
}

Dispatch::~Dispatch() { update_listeners(nullptr); }

void Dispatch::attach(gsl::not_null<IListener*> listener) {
	m_listeners.push_back(listener);
	listener->m_dispatch = this;
}

void Dispatch::detach(gsl::not_null<IListener*> listener) {
	listener->m_dispatch = nullptr;
	std::erase(m_listeners, listener);
}

void Dispatch::dispatch(std::span<Event const> events) const {
	for (auto const& event : events) {
		for (auto* listener : std::views::reverse(m_listeners)) {
			if (listener->consume_event(event)) { break; }
		}
	}
}

void Dispatch::update_listeners(Dispatch* target) const {
	for (auto* listener : m_listeners) { listener->m_dispatch = target; }
}
} // namespace le::event
