#pragma once
#include "le2d/input/mapping.hpp"
#include <functional>

namespace le::input {
class ListenerMapping : public IMapping {
  public:
	template <typename... Args>
	using Callback = std::move_only_function<void(Args const&...)>;

	void dispatch_events(std::span<le::Event const> events, Gamepad::Manager const& gamepads) final;
	void disengage_input() final;

	Callback<> on_window_close{};
	Callback<event::FramebufferResize> on_framebuffer_resize{};
	Callback<event::WindowResize> on_window_resize{};
	Callback<event::WindowFocus> on_window_focus{};
	Callback<event::CursorFocus> on_cursor_focus{};
	Callback<event::WindowIconify> on_window_iconify{};

	Callback<event::CursorPos> on_cursor_pos{};
	Callback<event::Codepoint> on_codepoint{};
	Callback<event::Key> on_key{};
	Callback<event::MouseButton> on_mouse_button{};
	Callback<event::Scroll> on_scroll{};
	Callback<event::Drop> on_drop{};

	Callback<> on_disengage{};

	Callback<Gamepad::Manager> update_gamepads{};
};
} // namespace le::input
