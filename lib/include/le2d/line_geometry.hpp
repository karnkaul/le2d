#pragma once
#include <kvf/ttf.hpp>
#include <le2d/vertex_array.hpp>
#include <string_view>

namespace le {
struct LineGeometry {
	using Codepoint = kvf::ttf::Codepoint;
	using Glyph = kvf::ttf::Glyph;

	[[nodiscard]] auto line_bounds(std::string_view line) const -> kvf::Rect<>;
	[[nodiscard]] auto next_glyph_position(std::string_view line) const -> glm::vec2;
	void write_line(VertexArray& out, std::string_view line);

	std::span<Glyph const> glyphs{};
	glm::vec2 position{};
	bool use_tofu{true};
};
} // namespace le
