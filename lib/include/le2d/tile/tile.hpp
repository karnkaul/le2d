#pragma once
#include <kvf/rect.hpp>
#include <le2d/tile/tile_id.hpp>

namespace le {
struct Tile {
	TileId id{};
	kvf::UvRect uv{kvf::uv_rect_v};
};
} // namespace le
