#include "le2d/input/listener_mapping.hpp"
#include <imgui.h>

namespace le::input {
namespace {
auto consume_if(auto& func, auto const&... args) -> bool {
	if (!func) { return false; }
	return func(args...);
}

void invoke_if(auto& func, auto const&... args) {
	if (!func) { return; }
	func(args...);
}
} // namespace

auto ListenerMapping::consume_event(event::WindowClose /*event*/) -> bool { return consume_if(on_window_close); }
auto ListenerMapping::consume_event(event::SwapchainResize const& event) -> bool { return consume_if(on_swapchain_resize, event); }
auto ListenerMapping::consume_event(event::WindowResize const& event) -> bool { return consume_if(on_window_resize, event); }
auto ListenerMapping::consume_event(event::WindowFocus event) -> bool { return consume_if(on_window_focus, event); }
auto ListenerMapping::consume_event(event::CursorFocus event) -> bool { return consume_if(on_cursor_focus, event); }
auto ListenerMapping::consume_event(event::WindowIconify event) -> bool { return consume_if(on_window_iconify, event); }
auto ListenerMapping::consume_event(event::CursorPos const& event) -> bool { return consume_if(on_cursor_pos, event); }
auto ListenerMapping::consume_event(event::Drop const& event) -> bool { return consume_if(on_drop, event); }
auto ListenerMapping::consume_event(event::Codepoint event) -> bool { return consume_if(on_codepoint, event); }
auto ListenerMapping::consume_event(event::Key const& event) -> bool { return consume_if(on_key, event); }
auto ListenerMapping::consume_event(event::MouseButton const& event) -> bool { return consume_if(on_mouse_button, event); }
auto ListenerMapping::consume_event(event::Scroll const& event) -> bool { return consume_if(on_scroll, event); }
auto ListenerMapping::consume_gamepads(Gamepad::Manager const& gamepads) -> bool { return consume_if(update_gamepads, gamepads); }

void ListenerMapping::dispatch_events() { invoke_if(on_dispatch); }
void ListenerMapping::disengage_input() { invoke_if(on_disengage); }
} // namespace le::input
