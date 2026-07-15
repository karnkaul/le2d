#pragma once
#include "le2d/input/mapping.hpp"
#include <functional>

namespace le::input {
class ListenerMapping : public IMapping {
  public:
	template <typename... Args>
	using Func = std::move_only_function<void(Args const&...)>;

	void dispatch_events(std::span<le::Event const> events, Gamepad::Manager const& gamepads) final;
	void disengage_input() final;

	Func<> on_window_close{};
	Func<event::FramebufferResize> on_framebuffer_resize{};
	Func<event::WindowResize> on_window_resize{};
	Func<event::WindowFocus> on_window_focus{};
	Func<event::CursorFocus> on_cursor_focus{};
	Func<event::WindowIconify> on_window_iconify{};

	Func<event::CursorPos> on_cursor_pos{};
	Func<event::Codepoint> on_codepoint{};
	Func<event::Key> on_key{};
	Func<event::MouseButton> on_mouse_button{};
	Func<event::Scroll> on_scroll{};
	Func<event::Drop> on_drop{};

	Func<> on_disengage{};

	Func<Gamepad::Manager> update_gamepads{};
};
} // namespace le::input
