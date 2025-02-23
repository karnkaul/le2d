#pragma once
#include <kvf/ttf.hpp>
#include <le2d/text_height.hpp>
#include <le2d/texture.hpp>
#include <le2d/vertex_array.hpp>
#include <unordered_map>

namespace le {
class FontAtlas {
  public:
	using Glyph = kvf::ttf::Glyph;
	using GlyphLayout = kvf::ttf::GlyphLayout;

	[[nodiscard]] static auto clamp(TextHeight height) -> TextHeight;

	explicit FontAtlas(gsl::not_null<kvf::RenderDevice*> render_device);

	auto build(gsl::not_null<kvf::ttf::Typeface*> face, TextHeight height) -> bool;

	[[nodiscard]] auto get_glyphs() const -> std::span<Glyph const> { return m_glyphs; }
	[[nodiscard]] auto get_texture() const -> Texture const& { return m_texture; }
	[[nodiscard]] auto get_height() const -> TextHeight { return m_height; }
	[[nodiscard]] auto get_face() const -> kvf::ttf::Typeface const* { return m_face; }

	auto push_layouts(std::vector<GlyphLayout>& out, std::string_view text, float n_line_height = 1.5f, bool use_tofu = true) const -> glm::vec2;

  private:
	kvf::ttf::Typeface* m_face{};
	Texture m_texture;
	std::vector<Glyph> m_glyphs{};
	TextHeight m_height{};
};

class Font {
  public:
	Font() = default;

	explicit Font(gsl::not_null<kvf::RenderDevice*> render_device, std::vector<std::byte> bytes = {});

	[[nodiscard]] auto is_loaded() const -> bool { return m_face.is_loaded(); }
	auto load_face(std::vector<std::byte> font_bytes) -> bool;

	[[nodiscard]] auto get_name() const -> klib::CString { return m_face.get_name(); }

	[[nodiscard]] auto get_atlas(TextHeight height) -> FontAtlas&;

  private:
	kvf::RenderDevice* m_render_device{};

	kvf::ttf::Typeface m_face;
	std::unordered_map<TextHeight, FontAtlas> m_atlases{};
};

void write_glyphs(VertexArray& out, std::span<kvf::ttf::GlyphLayout const> glyphs, glm::vec2 position = {}, kvf::Color color = kvf::white_v);
} // namespace le
