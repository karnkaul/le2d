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

	Text() = default;

	explicit Text(Font& font, std::string_view line, Params const& params = {}) { set_string(font, line, params); }

	void set_string(Font& font, std::string_view line, Params const& params = {});

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }
	[[nodiscard]] auto get_glyph_layouts() const -> std::span<kvf::ttf::GlyphLayout const> { return m_glyph_layouts; }

	[[nodiscard]] auto get_primitive() const -> Primitive;

  private:
	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	VertexArray m_vertices{};
	Texture const* m_texture{};
	glm::vec2 m_size{};
};

static_assert(ShapeT<Text>);
} // namespace shape
} // namespace le
