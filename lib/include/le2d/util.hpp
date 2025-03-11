#pragma once
#include <le2d/tile.hpp>
#include <string>
#include <vector>

namespace le::util {
[[nodiscard]] auto exe_path() -> std::string;

[[nodiscard]] auto divide_into_tiles(glm::vec2 size, int rows, int cols) -> std::vector<Tile>;
} // namespace le::util
