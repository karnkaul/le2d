#pragma once
#include <le2d/drawable/draw_primitive.hpp>
#include <le2d/font.hpp>
#include <le2d/text/text_geometry.hpp>

namespace le::drawable {
enum class TextExpand : std::int8_t { eBoth, eRight, eLeft };

struct TextParams {
	TextHeight height{TextHeight::Default};
	TextExpand expand{TextExpand::eBoth};
};

class TextBase : public IDrawPrimitive {
  public:
	using Params = TextParams;

	[[nodiscard]] auto to_primitive() const -> Primitive final;

	void set_string(Font& font, std::string_view line, Params const& params = {});

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }
	[[nodiscard]] auto get_texture() const -> Texture const* { return m_texture; }

  private:
	TextGeometry m_geometry{};
	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	Texture const* m_texture{};
	glm::vec2 m_size{};
};

class Text : public SingleDrawPrimitive<TextBase> {};
class InstancedText : public InstancedDrawPrimitive<TextBase> {};
} // namespace le::drawable
