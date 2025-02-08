#pragma once
#include <klib/polymorphic.hpp>
#include <kvf/time.hpp>
#include <le2d/event.hpp>
#include <le2d/renderer.hpp>
#include <ui/widget_state.hpp>

namespace hog::ui {
class Widget : public klib::Polymorphic {
  public:
	using State = WidgetState;

	[[nodiscard]] auto get_state() const -> State { return m_state; }

	void set_framebuffer_size(glm::vec2 framebuffer_size);

	void on_cursor(le::event::CursorPos const& cursor_pos);
	void on_button(le::event::MouseButton const& mouse_button);

	void set_disabled(bool disabled);

	[[nodiscard]] virtual auto get_hitbox() const -> kvf::Rect<> = 0;
	virtual void tick(kvf::Seconds dt);
	virtual void draw(le::Renderer& renderer) const = 0;

	virtual void on_click() {}
	virtual void disengage();

	kvf::Seconds click_debounce{100ms};

  protected:
	[[nodiscard]] auto is_cursor_hit() const -> bool;

	void on_mb1_press();
	void on_mb1_release();

	glm::vec2 m_framebuffer_size{};
	le::ndc::vec2 m_cursor_pos{};

	State m_state{};
	kvf::Seconds m_debounce_remain{};
};
} // namespace hog::ui
