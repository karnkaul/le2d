#pragma once
#include <kvf/time.hpp>
#include <le2d/drawables/drawable.hpp>
#include <le2d/event.hpp>
#include <le2d/line_input.hpp>
#include <le2d/shapes/text.hpp>

namespace le {
struct InputTextParams {
	TextHeight height{TextHeight::Default};
	char cursor_symbol{'|'};
	kvf::Color cursor_color{kvf::white_v};
	kvf::Seconds blink_period{1s};
};

namespace drawable {
class InputText : public IDrawable {
  public:
	using Params = InputTextParams;

	explicit InputText(gsl::not_null<Font*> font, Params const& params = {});

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }
	[[nodiscard]] auto get_atlas() const -> FontAtlas& { return m_line_input.get_atlas(); }

	[[nodiscard]] auto is_interactive() const -> bool { return m_interactive; }
	void set_interactive(bool interactive);

	[[nodiscard]] auto get_string() const -> std::string_view { return m_line_input.get_string(); }
	void set_string(std::string line);
	void write(char ch);
	void backspace();
	void delete_front();

	[[nodiscard]] auto get_cursor() const -> int { return m_line_input.get_cursor(); }
	void set_cursor(int cursor);
	void move_cursor(int const delta) { set_cursor(get_cursor() + delta); }
	void cursor_left() { move_cursor(-1); }
	void cursor_right() { move_cursor(+1); }
	void cursor_home() { set_cursor(0); }
	void cursor_end() { set_cursor(int(get_string().size())); }

	void on_key(event::Key const& key);
	void on_codepoint(event::Codepoint codepoint);

	void tick(kvf::Seconds dt);
	void draw(Renderer& renderer) const override;

	RenderInstance instance{};

  private:
	void update();
	void reset_blink();

	LineInput m_line_input;
	shape::Text m_cursor;
	kvf::Color m_cursor_color;
	kvf::Seconds m_blink_period;

	float m_cursor_offset_x{};

	glm::vec2 m_size{};

	kvf::Seconds m_elapsed{};
	float m_cursor_alpha{1.0f};
	bool m_interactive{true};
};
} // namespace drawable
} // namespace le
