#pragma once
#include <le2d/drawable/shape.hpp>
#include <le2d/input/controls.hpp>
#include <ui/widget.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace hog::ui {
class ScrollView : public le::input::Listener {
  public:
	enum class State : std::int8_t { Idle, Scrolling };

	auto consume_cursor_move(glm::vec2 pos) -> bool override;
	auto consume_mouse_button(le::event::MouseButton const& button) -> bool override;
	auto consume_scroll(le::event::Scroll const& scroll) -> bool override;

	void set_framebuffer_size(glm::vec2 size);
	void disengage();

	void add_widget(std::unique_ptr<Widget> widget);

	void reposition_widgets();

	void tick(kvf::Seconds dt);
	void draw(le::Renderer& renderer) const;

	[[nodiscard]] auto get_state() const -> State { return m_state; }

	le::drawable::Quad background{};
	float scroll_speed{30.0f};
	float scroll_threshold{5.0f};
	float y_pad{20.0f};

	float y_offset{};

  private:
	auto on_scroll(le::event::Scroll const& scroll) -> bool;
	auto on_mouse_button(le::event::MouseButton const& button) -> bool;
	auto on_cursor_pos(le::event::CursorPos const& cursor_pos) -> bool;

	void do_scroll(glm::vec2 cursor_pos);
	void move_widgets(float dy);

	void set_scissor(le::Renderer& renderer) const;

	std::vector<std::unique_ptr<Widget>> m_widgets{};
	glm::vec2 m_framebuffer_size{};
	glm::vec2 m_cursor_pos{};
	float m_prev_cursor_y{};
	std::optional<float> m_drag_start_y{};
	State m_state{};
	le::input::MouseButtonTrigger m_mb1_press{GLFW_MOUSE_BUTTON_1};
};
} // namespace hog::ui
