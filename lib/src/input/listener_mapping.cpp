#include "le2d/input/listener_mapping.hpp"
#include <imgui.h>

namespace le::input {
namespace {
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

void ListenerMapping::dispatch_events(std::span<le::Event const> events, Gamepad::Manager const& gamepads) {
	auto const visitor = klib::Visitor{
		[this](event::WindowClose const&) { invoke_if(Type::None, on_window_close); },
		[this](event::WindowFocus const& e) { invoke_if(Type::None, on_window_focus, e); },
		[this](event::CursorFocus const& e) { invoke_if(Type::Mouse, on_cursor_focus, e); },
		[this](event::WindowIconify const& e) { invoke_if(Type::None, on_window_iconify, e); },
		[this](event::WindowResize const& e) { invoke_if(Type::None, on_window_resize, e); },
		[this](event::FramebufferResize const& e) { invoke_if(Type::None, on_framebuffer_resize, e); },
		[this](event::CursorPos const& e) { invoke_if(Type::Mouse, on_cursor_pos, e); },
		[this](event::Codepoint const e) { invoke_if(Type::Keyboard, on_codepoint, e); },
		[this](event::Key const& e) { invoke_if(Type::Keyboard, on_key, e); },
		[this](event::MouseButton const& e) { invoke_if(Type::Mouse, on_mouse_button, e); },
		[this](event::Scroll const& e) { invoke_if(Type::Mouse, on_scroll, e); },
		[this](event::Drop const& e) { invoke_if(Type::None, on_drop, e); },
	};
	for (auto const& event : events) { std::visit(visitor, event); }

	invoke_if(Type::None, update_gamepads, gamepads);
}

void ListenerMapping::disengage_input() { invoke_if(Type::None, on_disengage); }
} // namespace le::input
