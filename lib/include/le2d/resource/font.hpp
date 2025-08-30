#pragma once
#include <kvf/ttf.hpp>
#include <le2d/resource/texture.hpp>
#include <le2d/text_height.hpp>

namespace le {
/// \brief Opaque interface for a Font Atlas.
class IFontAtlas : public IResource {
  public:
	using Glyph = kvf::ttf::Glyph;
	using GlyphLayout = kvf::ttf::GlyphLayout;

	virtual auto build(gsl::not_null<kvf::ttf::Typeface*> face, TextHeight height) -> bool = 0;

	[[nodiscard]] virtual auto get_glyphs() const -> std::span<Glyph const> = 0;
	[[nodiscard]] virtual auto get_texture() const -> ITexture const& = 0;
	[[nodiscard]] virtual auto get_height() const -> TextHeight = 0;

	virtual auto push_layouts(std::vector<GlyphLayout>& out, std::string_view text, float n_line_height = 1.5f, bool use_tofu = true) const -> glm::vec2 = 0;
};

/// \brief Opaque interface for a Font.
class IFont : public IResource {
  public:
	virtual auto load_face(std::vector<std::byte> font_bytes) -> bool = 0;

	[[nodiscard]] virtual auto get_name() const -> klib::CString = 0;

	[[nodiscard]] virtual auto get_atlas(TextHeight height) -> IFontAtlas& = 0;
};
} // namespace le
