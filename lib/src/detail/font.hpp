#pragma once
#include <detail/texture.hpp>
#include <klib/assert.hpp>
#include <le2d/resource/font.hpp>
#include <le2d/text_height.hpp>
#include <algorithm>
#include <unordered_map>

namespace le {
inline auto IFontAtlas::clamp(TextHeight height) -> TextHeight { return std::clamp(height, TextHeight::Min, TextHeight::Max); }
} // namespace le

namespace le::detail {
class FontAtlas : public IFontAtlas {
  public:
	using Glyph = kvf::ttf::Glyph;
	using GlyphLayout = kvf::ttf::GlyphLayout;

	explicit FontAtlas(gsl::not_null<kvf::IRenderApi const*> render_api) : m_texture(render_api) {}

	[[nodiscard]] auto is_ready() const -> bool final { return m_texture.is_ready() && m_face && m_face->is_loaded(); }

	auto build(gsl::not_null<kvf::ttf::Typeface*> face, TextHeight height) -> bool final {
		if (!m_texture.is_ready()) { return false; }

		height = clamp(height);
		auto ttf_atlas = face->build_atlas(std::uint32_t(height));
		m_texture.overwrite(ttf_atlas.bitmap.bitmap());

		m_face = face;
		m_height = height;
		m_glyphs = std::move(ttf_atlas.glyphs);

		return true;
	}

	[[nodiscard]] auto get_glyphs() const -> std::span<Glyph const> final { return m_glyphs; }
	[[nodiscard]] auto get_texture() const -> ITexture2 const& final { return m_texture; }
	[[nodiscard]] auto get_height() const -> TextHeight final { return m_height; }

	auto push_layouts(std::vector<GlyphLayout>& out, std::string_view const text, float const n_line_height, bool const use_tofu) const -> glm::vec2 final {
		if (!is_ready()) { return {}; }
		auto const input = kvf::ttf::TextInput{
			.text = text,
			.glyphs = m_glyphs,
			.height = std::uint32_t(m_height),
			.n_line_height = n_line_height,
		};
		return m_face->push_layouts(out, input, use_tofu);
	}

  private:
	kvf::ttf::Typeface* m_face{};
	Texture m_texture;
	std::vector<Glyph> m_glyphs{};
	TextHeight m_height{};
};

class Font : public IFont {
  public:
	explicit Font(gsl::not_null<kvf::IRenderApi const*> render_api) : m_render_api(render_api) {}

	[[nodiscard]] auto is_ready() const -> bool final { return m_face.is_loaded(); }

	auto load_face(std::vector<std::byte> font_bytes) -> bool final {
		auto face = kvf::ttf::Typeface{std::move(font_bytes)};
		if (!face) { return false; }

		m_face = std::move(face);
		m_atlases.clear();

		return true;
	}

	[[nodiscard]] auto get_name() const -> klib::CString final { return m_face.get_name(); }

	[[nodiscard]] auto get_atlas(TextHeight height) -> FontAtlas& final {
		KLIB_ASSERT(is_ready());
		height = FontAtlas::clamp(height);
		auto it = m_atlases.find(height);
		if (it == m_atlases.end()) {
			auto atlas = FontAtlas{m_render_api};
			atlas.build(&m_face, height);
			it = m_atlases.insert({height, std::move(atlas)}).first;
		}
		return it->second;
	}

  private:
	gsl::not_null<kvf::IRenderApi const*> m_render_api;

	kvf::ttf::Typeface m_face{};
	std::unordered_map<TextHeight, FontAtlas> m_atlases{};
};

// void write_glyphs(VertexArray& out, std::span<kvf::ttf::GlyphLayout const> glyphs, glm::vec2 position = {}, kvf::Color color = kvf::white_v);
} // namespace le::detail
