#pragma once
#include <le2d/anim/timeline.hpp>
#include <le2d/tile/tile.hpp>
#include <string>
#include <vector>

namespace le::util {
[[nodiscard]] auto exe_path() -> std::string;

[[nodiscard]] auto divide_into_tiles(int rows, int cols) -> std::vector<Tile>;

[[nodiscard]] auto generate_flipbook_timeline(std::span<Tile const> tiles, kvf::Seconds duration) -> anim::Timeline<TileId>;
} // namespace le::util
