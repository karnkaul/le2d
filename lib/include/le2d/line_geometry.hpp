#pragma once
#include <kvf/ttf.hpp>
#include <le2d/vertex_array.hpp>
#include <string_view>

namespace le {
class FontAtlas;

struct LineGeometry {
	using Codepoint = kvf::ttf::Codepoint;
	using Glyph = kvf::ttf::Glyph;
	using GlyphLayout = kvf::ttf::GlyphLayout;

	[[nodiscard]] auto line_bounds(std::vector<GlyphLayout>& buf, std::string_view line) const -> kvf::Rect<>;
	void write_line(VertexArray& out, std::vector<GlyphLayout>& buf, std::string_view line);

	gsl::not_null<FontAtlas const*> atlas;
	glm::vec2 position{};
	bool use_tofu{true};
};
} // namespace le
