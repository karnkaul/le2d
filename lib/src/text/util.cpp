#include <kvf/is_positive.hpp>
#include <le2d/shape/quad.hpp>
#include <le2d/text/util.hpp>
#include <algorithm>

namespace le {
auto util::clamp(TextHeight height) -> TextHeight { return std::clamp(height, TextHeight::Min, TextHeight::Max); }

void util::write_glyphs(VertexArray& out, std::span<kvf::ttf::GlyphLayout const> glyphs, glm::vec2 const position, kvf::Color const color) {
	out.reserve(glyphs.size() * shape::Quad::vertex_count_v, glyphs.size() * shape::Quad::indices_v.size());
	for (auto const& layout : glyphs) {
		if (!kvf::is_positive(layout.glyph->size)) { continue; }

		auto quad = shape::Quad{};
		quad.create(layout.glyph->rect(position + layout.baseline), layout.glyph->uv_rect, color);
		out.append(quad.get_vertices(), shape::Quad::indices_v);
	}
}
} // namespace le
