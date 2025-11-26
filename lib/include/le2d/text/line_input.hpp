#pragma once
#include "le2d/primitive.hpp"
#include "le2d/resource/font.hpp"
#include "le2d/text/text_geometry.hpp"
#include <gsl/pointers>

namespace le {
/// \brief Interactive text input for a single line.
class LineInput {
  public:
	explicit LineInput(gsl::not_null<IFont*> font, TextHeight height = TextHeight::Default);

	[[nodiscard]] auto get_string() const -> std::string_view { return m_line; }
	[[nodiscard]] auto get_height() const -> TextHeight { return m_atlas->get_height(); }
	[[nodiscard]] auto get_cursor() const -> int { return m_cursor; }
	[[nodiscard]] auto get_cursor_x() const -> float { return m_cursor_x; }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }
	[[nodiscard]] auto get_glyph_layouts() const -> std::span<kvf::ttf::GlyphLayout const> { return m_glyph_layouts; }

	[[nodiscard]] auto get_atlas() const -> IFontAtlas& { return *m_atlas; }
	[[nodiscard]] auto get_texture() const -> ITexture const& { return get_atlas().get_texture(); }

	[[nodiscard]] auto to_primitive() const -> Primitive;

	void set_string(std::string line);
	void append(std::string_view str);

	void write(char ch);
	void backspace();
	void delete_front();

	void set_cursor(int cursor);
	void move_cursor(int delta);

  protected:
	void update();

  private:
	void update_cursor_x();

	gsl::not_null<IFontAtlas*> m_atlas;

	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	TextGeometry m_geometry{};

	std::string m_line{};
	glm::vec2 m_size{};
	int m_cursor{};
	float m_cursor_x{};
	float m_next_glyph_x{};
};
} // namespace le
