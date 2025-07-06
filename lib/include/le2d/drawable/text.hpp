#pragma once
#include <le2d/drawable/draw_primitive.hpp>
#include <le2d/resource/font.hpp>
#include <le2d/text/text_geometry.hpp>

namespace le::drawable {
/// \brief Horizontal text expansion.
enum class TextExpand : std::int8_t { eBoth, eRight, eLeft };

/// \brief Text generation parameters.
struct TextParams {
	TextHeight height{TextHeight::Default};
	TextExpand expand{TextExpand::eBoth};
};

/// \brief Base class for Text types.
class TextBase : public IDrawPrimitive {
  public:
	using Params = TextParams;

	[[nodiscard]] auto to_primitive() const -> Primitive final;

	void set_string(IFont& font, std::string_view line, Params const& params = {});

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }
	[[nodiscard]] auto get_texture() const -> ITexture2 const* { return m_texture; }

  private:
	TextGeometry m_geometry{};
	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	ITexture2 const* m_texture{};
	glm::vec2 m_size{};
};

/// \brief Text Draw Primitive.
class Text : public SingleDrawPrimitive<TextBase> {};
/// \brief Instanced Text Draw Primitive.
class InstancedText : public InstancedDrawPrimitive<TextBase> {};
} // namespace le::drawable
