#pragma once
#include <kvf/bitmap.hpp>
#include <kvf/color.hpp>
#include <le2d/anim/timeline.hpp>
#include <le2d/tile/tile.hpp>
#include <bit>
#include <string>
#include <vector>

namespace le::util {
inline constexpr auto white_pixel_bytes_v = std::bit_cast<std::array<std::byte, sizeof(kvf::Color)>>(kvf::white_v);
inline constexpr auto white_bitmap_v = kvf::Bitmap{.bytes = white_pixel_bytes_v, .size = {1, 1}};

[[nodiscard]] auto exe_path() -> std::string;

[[nodiscard]] auto divide_into_tiles(int rows, int cols) -> std::vector<Tile>;

[[nodiscard]] auto generate_flipbook_timeline(std::span<Tile const> tiles, kvf::Seconds duration) -> anim::Timeline<TileId>;
} // namespace le::util
