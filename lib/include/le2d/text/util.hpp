#pragma once
#include <le2d/resource/font.hpp>
#include <le2d/vertex_array.hpp>

namespace le::util {
[[nodiscard]] auto clamp(TextHeight height) -> TextHeight;
void write_glyphs(VertexArray& out, std::span<kvf::ttf::GlyphLayout const> glyphs, glm::vec2 position = {}, kvf::Color color = kvf::white_v);
} // namespace le::util
