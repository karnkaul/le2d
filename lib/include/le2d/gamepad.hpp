#pragma once
#include <GLFW/glfw3.h>
#include <klib/c_string.hpp>

namespace le {
class Gamepad {
  public:
	[[nodiscard]] static auto is_connected(int id) -> bool;
	[[nodiscard]] static auto get_active() -> Gamepad;
	[[nodiscard]] static auto get_by_id(int id) -> Gamepad;

	[[nodiscard]] auto get_id() const -> int { return m_id; }
	[[nodiscard]] auto is_connected() const -> bool { return is_connected(get_id()); }
	[[nodiscard]] auto get_name() const -> klib::CString;

	[[nodiscard]] auto any_button_pressed() const -> bool;
	[[nodiscard]] auto is_pressed(int button) const -> bool;
	[[nodiscard]] auto get_axis(int axis) const -> float;

  private:
	static auto set_state(Gamepad& out_pad) -> bool;

	GLFWgamepadstate m_state{};
	int m_id{-1};
};
} // namespace le
