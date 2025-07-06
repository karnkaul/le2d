#pragma once
#include <kvf/ttf.hpp>
#include <le2d/resource/resource.hpp>
#include <le2d/resource/texture.hpp>
#include <le2d/text_height.hpp>
#include <le2d/vertex_array.hpp>
#include <memory>

namespace le {
class IFontAtlas : public IResource {
  public:
	using Glyph = kvf::ttf::Glyph;
	using GlyphLayout = kvf::ttf::GlyphLayout;

	[[nodiscard]] static auto clamp(TextHeight height) -> TextHeight;

	virtual auto build(gsl::not_null<kvf::ttf::Typeface*> face, TextHeight height) -> bool = 0;

	[[nodiscard]] virtual auto get_glyphs() const -> std::span<Glyph const> = 0;
	[[nodiscard]] virtual auto get_texture() const -> std::shared_ptr<ITexture2 const> = 0;
	[[nodiscard]] virtual auto get_height() const -> TextHeight = 0;

	virtual auto push_layouts(std::vector<GlyphLayout>& out, std::string_view text, float n_line_height = 1.5f, bool use_tofu = true) const -> glm::vec2 = 0;
};

class IFont : public IResource {
  public:
	virtual auto load_face(std::vector<std::byte> font_bytes) -> bool = 0;

	[[nodiscard]] virtual auto get_name() const -> klib::CString = 0;

	[[nodiscard]] virtual auto get_atlas(TextHeight height) -> IFontAtlas& = 0;
};

// void write_glyphs(VertexArray& out, std::span<kvf::ttf::GlyphLayout const> glyphs, glm::vec2 position = {}, kvf::Color color = kvf::white_v);
} // namespace le
