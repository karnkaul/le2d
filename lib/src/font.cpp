#include <klib/assert.hpp>
#include <le2d/font.hpp>
#include <algorithm>

namespace le {
auto FontAtlas::clamp(TextHeight const height) -> TextHeight { return std::clamp(height, TextHeight::Min, TextHeight::Max); }

FontAtlas::FontAtlas(gsl::not_null<kvf::RenderDevice*> render_device) : m_texture(render_device) {}

auto FontAtlas::build(kvf::ttf::Typeface& face, TextHeight height) -> bool {
	if (!face.is_loaded()) { return false; }

	height = clamp(height);
	face.set_height(std::uint32_t(height));
	auto ttf_atlas = face.build_atlas();
	if (!m_texture.write(ttf_atlas.bitmap.bitmap())) { return false; }

	m_height = height;
	m_glyphs = std::move(ttf_atlas.glyphs);

	return true;
}

Font::Font(gsl::not_null<kvf::RenderDevice*> render_device, std::vector<std::byte> bytes) : m_render_device(render_device) { load_face(std::move(bytes)); }

auto Font::load_face(std::vector<std::byte> font_bytes) -> bool {
	auto face = kvf::ttf::Typeface{std::move(font_bytes)};
	if (!face) { return false; }

	m_face = std::move(face);
	m_atlases.clear();

	return true;
}

auto Font::get_atlas(TextHeight height) -> FontAtlas& {
	height = FontAtlas::clamp(height);
	auto it = m_atlases.find(height);
	if (it == m_atlases.end()) {
		auto atlas = FontAtlas{m_render_device};
		atlas.build(m_face, height);
		it = m_atlases.insert({height, std::move(atlas)}).first;
	}
	return it->second;
}
} // namespace le
