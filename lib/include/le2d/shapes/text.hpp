#pragma once
#include <le2d/font.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/shapes/shape.hpp>
#include <le2d/vertex_array.hpp>
#include <gsl/pointers>

namespace le {
enum class TextExpand : std::int8_t { eBoth, eRight, eLeft };

struct TextParams {
	TextHeight height{TextHeight::Default};
	TextExpand expand{TextExpand::eBoth};
};

namespace shape {
class Text {
  public:
	using Params = TextParams;

	explicit Text(gsl::not_null<Font*> font) : m_font(font) {}

	[[nodiscard]] auto get_font() const -> Font& { return *m_font; }

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }

	void set_string(std::string_view line, Params const& params = {});

	[[nodiscard]] auto get_primitive() const -> Primitive;

  private:
	Font* m_font;

	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	VertexArray m_vertices{};
	Texture const* m_texture{};
	glm::vec2 m_size{};
};

static_assert(ShapeT<Text>);
} // namespace shape
} // namespace le
