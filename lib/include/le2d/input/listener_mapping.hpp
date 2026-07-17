#pragma once
#include "le2d/input/mapping.hpp"
#include <functional>

namespace le::input {
/// \brief Routes events to callback members.
/// All callbacks are optional.
class ListenerMapping : public IMapping {
  public:
	template <typename... Args>
	using ConsumeCallback = std::move_only_function<bool(Args const&...)>;
	template <typename... Args>
	using Callback = std::move_only_function<void(Args const&...)>;

	auto consume_event(event::WindowClose event) -> bool final;
	auto consume_event(event::SwapchainResize const& event) -> bool final;
	auto consume_event(event::WindowResize const& event) -> bool final;
	auto consume_event(event::WindowFocus event) -> bool final;
	auto consume_event(event::CursorFocus event) -> bool final;
	auto consume_event(event::WindowIconify event) -> bool final;
	auto consume_event(event::Drop const& event) -> bool final;
	auto consume_event(event::CursorPos const& event) -> bool final;

	auto consume_event(event::Codepoint event) -> bool final;
	auto consume_event(event::Key const& event) -> bool final;
	auto consume_event(event::MouseButton const& event) -> bool final;
	auto consume_event(event::Scroll const& event) -> bool final;

	auto consume_gamepads(Gamepad::Manager const& gamepads) -> bool final;

	void dispatch_events() final;
	void disengage_input() final;

	ConsumeCallback<> on_window_close{};
	ConsumeCallback<event::SwapchainResize> on_swapchain_resize{};
	ConsumeCallback<event::WindowResize> on_window_resize{};
	ConsumeCallback<event::WindowFocus> on_window_focus{};
	ConsumeCallback<event::CursorFocus> on_cursor_focus{};
	ConsumeCallback<event::WindowIconify> on_window_iconify{};
	ConsumeCallback<event::Drop> on_drop{};

	ConsumeCallback<event::CursorPos> on_cursor_pos{};
	ConsumeCallback<event::Codepoint> on_codepoint{};
	ConsumeCallback<event::Key> on_key{};
	ConsumeCallback<event::MouseButton> on_mouse_button{};
	ConsumeCallback<event::Scroll> on_scroll{};

	ConsumeCallback<Gamepad::Manager> update_gamepads{};

	Callback<> on_dispatch{};
	Callback<> on_disengage{};
};
} // namespace le::input
