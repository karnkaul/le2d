#include "le2d/input/router.hpp"
#include <imgui.h>
#include <ranges>

namespace le::input {
namespace {
template <typename... Ts>
constexpr auto holds_any_of(Event const& e) {
	return (... || std::holds_alternative<Ts>(e));
}

enum class Type : std::int8_t { None, Keyboard, Mouse };

void invoke_if(Type const type, auto& func, auto const&... args) {
	switch (type) {
	default:
	case Type::None: break;
	case Type::Keyboard: {
		if (ImGui::GetIO().WantCaptureKeyboard) { return; }
		break;
	}
	case Type::Mouse: {
		if (ImGui::GetIO().WantCaptureMouse) { return; }
		break;
	}
	}

	if (!func) { return; }
	func(args...);
}
} // namespace

void Router::push_mapping(std::shared_ptr<IMapping> const& mapping) {
	if (!mapping || (!m_mappings.empty() && m_mappings.back().lock() == mapping)) { return; }
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

auto Router::contains_mapping(std::shared_ptr<IMapping> const& mapping) const -> bool {
	return std::ranges::any_of(m_mappings, [&mapping](auto const& ptr) { return ptr.lock() == mapping; });
}

void Router::set_terminal_mapping(std::shared_ptr<IMapping> const& mapping) { m_terminal_mapping = mapping; }

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
		dispatch_events(events);
	}
}

void Router::disengage() {
	cascade_invoke_if([](IMapping& mapping) {
		mapping.disengage_input();
		return true;
	});
}

template <typename F>
auto Router::cascade_invoke_if(F func) const -> bool {
	auto const pred = [&](std::weak_ptr<IMapping> const& ptr) {
		auto mapping = ptr.lock();
		return !mapping || func(*mapping);
	};
	return std::ranges::all_of(std::views::reverse(m_mappings), pred);
}

void Router::pre_dispatch() {
	m_mappings_buffer.clear();
	m_mappings_buffer.reserve(m_mappings.size() + 1);
	std::erase_if(m_mappings, [this](auto const& ptr) {
		auto mapping = ptr.lock();
		if (!mapping) { return true; }
		m_mappings_buffer.push_back(std::move(mapping));
		return false;
	});
	if (auto terminal_mapping = m_terminal_mapping.lock()) { m_mappings_buffer.push_back(std::move(terminal_mapping)); }
}

void Router::dispatch_events(std::span<Event const> events) {
	for (auto const& event : events) {
		auto const imgui_wants = klib::Visitor{
			[](auto const&) { return false; },
			[](event::CursorPos const&) { return ImGui::GetIO().WantCaptureMouse; },
			[](event::Codepoint const) { return ImGui::GetIO().WantCaptureKeyboard; },
			[](event::Key const&) { return ImGui::GetIO().WantCaptureKeyboard; },
			[](event::MouseButton const&) { return ImGui::GetIO().WantCaptureMouse; },
			[](event::Scroll const&) { return ImGui::GetIO().WantCaptureMouse; },
		};
		if (std::visit(imgui_wants, event)) { continue; }

		auto disengage_remaining = false;

		for (auto const& mapping : std::views::reverse(m_mappings_buffer)) {
			if (disengage_remaining) {
				mapping->disengage_input();
				continue;
			}

			disengage_remaining = std::visit([&mapping](auto const& e) { return mapping->consume_event(e); }, event);
		}
	}

	auto disengage_remaining = false;
	for (auto const& mapping : std::views::reverse(m_mappings_buffer)) {
		if (disengage_remaining) {
			mapping->disengage_input();
			continue;
		}

		disengage_remaining = mapping->consume_gamepads(m_gamepad_manager);

		mapping->dispatch_events();
	}
}
} // namespace le::input
