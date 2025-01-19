#include <kvf/is_positive.hpp>
#include <le2d/font.hpp>
#include <le2d/line_geometry.hpp>
#include <le2d/shapes/quad.hpp>

namespace le {
namespace {
using LineLayout = kvf::ttf::LineLayout;

[[nodiscard]] auto to_layout(LineGeometry const& line_geometry) {
	return LineLayout{
		.face = line_geometry.atlas->get_face(),
		.glyphs = line_geometry.atlas->get_glyphs(),
		.use_tofu = line_geometry.use_tofu,
	};
}
} // namespace

auto LineGeometry::line_bounds(std::vector<GlyphLayout>& buf, std::string_view const line) const -> kvf::Rect<> {
	buf.clear();
	buf.reserve(line.size());
	to_layout(*this).generate(buf, line, position);
	return kvf::ttf::glyph_bounds(buf);
}

void LineGeometry::write_line(VertexArray& out, std::vector<GlyphLayout>& buf, std::string_view const line) {
	buf.clear();
	buf.reserve(line.size());
	position = to_layout(*this).generate(buf, line, position);
	for (auto const& entry : buf) {
		if (!kvf::is_positive(entry.glyph->size)) { continue; }

		auto const rect = entry.glyph->rect(entry.baseline);
		auto const quad = shape::Quad{rect, entry.glyph->uv_rect};
		out.append(quad.get_vertices(), shape::Quad::indices_v);
	}
}
} // namespace le
