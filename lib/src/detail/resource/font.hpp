#pragma once
#include "detail/resource/texture.hpp"
#include "klib/debug/assert.hpp"
#include "le2d/resource/font.hpp"
#include "le2d/text/util.hpp"
#include "le2d/text_height.hpp"
#include <unordered_map>

namespace le::detail {
class FontAtlas : public IFontAtlas {
  public:
	using Glyph = kvf::ttf::Glyph;
	using GlyphLayout = kvf::ttf::GlyphLayout;

	explicit FontAtlas(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory)
		: m_texture(render_device, sampler_factory) {}

	void build(gsl::not_null<kvf::ttf::Typeface*> face, TextHeight height) {
		height = util::clamp(height);
		auto ttf_atlas = face->build_atlas(std::uint32_t(height));
		m_texture.overwrite(ttf_atlas.bitmap.bitmap());

		m_face = face;
		m_height = height;
		m_glyphs = std::move(ttf_atlas.glyphs);
	}

  private:
	[[nodiscard]] auto get_glyphs() const -> std::span<Glyph const> final { return m_glyphs; }
	[[nodiscard]] auto get_texture() const -> ITexture const& final { return m_texture; }
	[[nodiscard]] auto get_height() const -> TextHeight final { return m_height; }

	auto push_layouts(std::vector<GlyphLayout>& out, std::string_view const text, float const n_line_height, bool const use_tofu) const -> glm::vec2 final {
		auto const input = kvf::ttf::TextInput{
			.text = text,
			.glyphs = m_glyphs,
			.height = std::uint32_t(m_height),
			.n_line_height = n_line_height,
		};
		return m_face->push_layouts(out, input, use_tofu);
	}

	kvf::ttf::Typeface* m_face{};
	Texture m_texture;
	std::vector<Glyph> m_glyphs{};
	TextHeight m_height{};
};

class Font : public IFont {
  public:
	explicit Font(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory)
		: m_render_device(render_device), m_sampler_factory(sampler_factory) {}

	auto load_face(std::vector<std::byte> font_bytes) -> bool final {
		auto face = kvf::ttf::Typeface{std::move(font_bytes)};
		if (!face) { return false; }

		m_face = std::move(face);
		m_atlases.clear();

		return true;
	}

	[[nodiscard]] auto get_name() const -> klib::CString final {
		KLIB_ASSERT(m_face.is_loaded());
		return m_face.get_name();
	}

	[[nodiscard]] auto get_atlas(TextHeight height) -> FontAtlas& final {
		KLIB_ASSERT(m_face.is_loaded());
		height = util::clamp(height);
		auto it = m_atlases.find(height);
		if (it == m_atlases.end()) {
			auto atlas = FontAtlas{m_render_device, m_sampler_factory};
			atlas.build(&m_face, height);
			it = m_atlases.insert({height, std::move(atlas)}).first;
		}
		return it->second;
	}

  private:
	gsl::not_null<kvf::IRenderDevice*> m_render_device;
	gsl::not_null<ISamplerFactory*> m_sampler_factory;

	kvf::ttf::Typeface m_face{};
	std::unordered_map<TextHeight, FontAtlas> m_atlases{};
};
} // namespace le::detail
