#include <le2d/input/dispatch.hpp>
#include <functional>
#include <ranges>

namespace le::input {
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

void Dispatch::attach(gsl::not_null<Listener*> listener) {
	m_listeners.push_back(listener);
	listener->m_dispatch = this;
}

void Dispatch::detach(gsl::not_null<Listener*> listener) {
	listener->m_dispatch = nullptr;
	std::erase(m_listeners, listener);
}

void Dispatch::on_cursor_move(glm::vec2 pos) { dispatch(&Listener::consume_cursor_move, pos); }

void Dispatch::on_codepoint(event::Codepoint codepoint) { dispatch(&Listener::consume_codepoint, codepoint); }

void Dispatch::on_key(event::Key const& key) { dispatch(&Listener::consume_key, key); }

void Dispatch::on_mouse_button(event::MouseButton const& button) { dispatch(&Listener::consume_mouse_button, button); }

void Dispatch::on_scroll(event::Scroll const& scroll) { dispatch(&Listener::consume_scroll, scroll); }

void Dispatch::on_drop(event::Drop const& drop) { dispatch(&Listener::consume_drop, drop); }

void Dispatch::update_listeners(Dispatch* target) const {
	for (auto* listener : m_listeners) { listener->m_dispatch = target; }
}

template <typename FPtr, typename T>
void Dispatch::dispatch(FPtr const fptr, T const& event) const {
	for (auto* listener : std::views::reverse(m_listeners)) {
		if (std::invoke(fptr, listener, event)) { break; }
	}
}
} // namespace le::input
