#pragma once
#include <klib/polymorphic.hpp>
#include <kvf/time.hpp>
#include <le2d/input/listener.hpp>
#include <le2d/renderer.hpp>
#include <ui/widget_state.hpp>

namespace hog::ui {
class Widget : public le::input::Listener {
  public:
	using State = WidgetState;

	[[nodiscard]] auto get_state() const -> State { return m_state; }

	auto consume_cursor_move(glm::vec2 pos) -> bool override;
	auto consume_mouse_button(le::event::MouseButton const& button) -> bool override;

	void set_disabled(bool disabled);

	[[nodiscard]] virtual auto get_hitbox() const -> kvf::Rect<> = 0;
	virtual void tick(kvf::Seconds dt);
	virtual void draw(le::Renderer& renderer) const = 0;

	virtual void on_click() {}
	virtual void disengage();

	[[nodiscard]] virtual auto get_position() const -> glm::vec2 = 0;
	virtual void set_position(glm::vec2 position) = 0;

	kvf::Seconds click_debounce{100ms};

  protected:
	[[nodiscard]] auto is_cursor_hit() const -> bool;

	void on_cursor(le::event::CursorPos const& cursor_pos);
	auto on_button(le::event::MouseButton const& mouse_button) -> bool;

	glm::vec2 m_cursor_pos{};

	State m_state{};
	kvf::Seconds m_debounce_remain{};
};
} // namespace hog::ui
