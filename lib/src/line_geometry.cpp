#include <kvf/is_positive.hpp>
#include <le2d/line_geometry.hpp>
#include <le2d/shapes/quad.hpp>

namespace le {
namespace {
using GlyphIterator = kvf::ttf::GlyphIterator;

[[nodiscard]] auto to_iterator(LineGeometry const& line_geometry) {
	return GlyphIterator{
		.glyphs = line_geometry.glyphs,
		.use_tofu = line_geometry.use_tofu,
	};
}
} // namespace

auto LineGeometry::line_bounds(std::string_view const line) const -> kvf::Rect<> {
	auto ret = to_iterator(*this).line_bounds(line);
	ret.lt += position;
	ret.rb += position;
	return ret;
}

auto LineGeometry::next_glyph_position(std::string_view const line) const -> glm::vec2 { return position + to_iterator(*this).next_glyph_position(line); }

void LineGeometry::write_line(VertexArray& out, std::string_view const line) {
	auto iterator = to_iterator(*this);
	iterator.iterate(line, [&](GlyphIterator::Entry const& entry) {
		if (kvf::is_positive(entry.glyph->size)) {
			auto const rect = entry.glyph->rect(position);
			auto const quad = shape::Quad{rect, entry.glyph->uv_rect};
			out.append(quad.get_vertices(), shape::Quad::indices_v);
		}
		position = GlyphIterator::advance(position, entry);
	});
}
} // namespace le
