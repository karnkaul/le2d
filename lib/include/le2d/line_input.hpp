#pragma once
#include <klib/polymorphic.hpp>
#include <le2d/font.hpp>
#include <le2d/vertex_array.hpp>
#include <gsl/pointers>

namespace le {
class LineInput : public klib::Polymorphic {
  public:
	explicit LineInput(gsl::not_null<Font*> font, TextHeight height = TextHeight::Default);

	[[nodiscard]] auto get_string() const -> std::string_view { return m_line; }
	[[nodiscard]] auto get_height() const -> TextHeight { return m_atlas->get_height(); }
	[[nodiscard]] auto get_cursor() const -> int { return m_cursor; }
	[[nodiscard]] auto get_cursor_x() const -> float { return m_cursor_x; }

	[[nodiscard]] auto get_vertices() const -> VertexArray const& { return m_vertices; }
	[[nodiscard]] auto get_atlas() const -> FontAtlas& { return *m_atlas; }
	[[nodiscard]] auto get_texture() const -> Texture const& { return get_atlas().get_texture(); }

	void set_string(std::string line);

	void write(char ch);
	void backspace();
	void delete_front();

	void set_cursor(int cursor);
	void move_cursor(int delta);

  protected:
	virtual void update();

  private:
	void update_cursor_x();

	gsl::not_null<FontAtlas*> m_atlas;

	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	VertexArray m_vertices{};

	std::string m_line{};
	int m_cursor{};
	float m_cursor_x{};
	float m_next_glyph_x{};
};
} // namespace le
