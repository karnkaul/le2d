#include "le2d/input/dispatch.hpp"
#include <imgui.h>
#include <klib/assert.hpp>
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
	KLIB_ASSERT_DEBUG(std::ranges::find(m_listeners, listener.get()) == m_listeners.end());
	m_listeners.push_back(listener);
	listener->m_dispatch = this;
}

void Dispatch::detach(gsl::not_null<Listener*> listener) {
	listener->m_dispatch = nullptr;
	std::erase(m_listeners, listener);
}

void Dispatch::on_cursor_move(glm::vec2 pos) { dispatch(&Listener::consume_cursor_move, pos, Type::Mouse); }

void Dispatch::on_codepoint(event::Codepoint codepoint) { dispatch(&Listener::consume_codepoint, codepoint, Type::Keyboard); }

void Dispatch::on_key(event::Key const& key) { dispatch(&Listener::consume_key, key, Type::Keyboard); }

void Dispatch::on_mouse_button(event::MouseButton const& button) { dispatch(&Listener::consume_mouse_button, button, Type::Mouse); }

void Dispatch::on_scroll(event::Scroll const& scroll) { dispatch(&Listener::consume_scroll, scroll, Type::Mouse); }

void Dispatch::on_drop(event::Drop const& drop) { dispatch(&Listener::consume_drop, drop, Type::None); }

void Dispatch::handle_events(glm::ivec2 const framebuffer_size, std::span<le::Event const> events) {
	auto const visitor = klib::SubVisitor{
		[this, framebuffer_size](le::event::CursorPos const& e) { on_cursor_move(e.normalized.to_target(framebuffer_size)); },
		[this](le::event::Codepoint const e) { on_codepoint(e); },
		[this](le::event::Key const& e) { on_key(e); },
		[this](le::event::MouseButton const& e) { on_mouse_button(e); },
		[this](le::event::Scroll const& e) { on_scroll(e); },
		[this](le::event::Drop const& e) { on_drop(e); },
	};
	for (auto const& event : events) { std::visit(visitor, event); }
}

void Dispatch::disengage_all() {
	for (auto* listener : m_listeners) { listener->disengage_input(); }
}

void Dispatch::update_listeners(Dispatch* target) const {
	for (auto* listener : m_listeners) { listener->m_dispatch = target; }
}

template <typename FPtr, typename T>
void Dispatch::dispatch(FPtr const fptr, T const& event, Type const type) const {
	if (honor_imgui_want_capture) {
		switch (type) {
		case Type::Keyboard: {
			if (ImGui::GetIO().WantCaptureKeyboard) { return; }
			break;
		}
		case Type::Mouse: {
			if (ImGui::GetIO().WantCaptureMouse) { return; }
			break;
		}
		case Type::None: break;
		}
	}

	auto consumed = false;
	for (auto* listener : std::views::reverse(m_listeners)) {
		if (!consumed) {
			consumed = std::invoke(fptr, listener, event);
		} else {
			listener->disengage_input();
		}
	}
}
} // namespace le::input
