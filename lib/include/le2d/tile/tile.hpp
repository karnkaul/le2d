#pragma once
#include <kvf/rect.hpp>
#include <le2d/tile/tile_id.hpp>

namespace le {
/// \brief Tile in a sprite sheet.
struct Tile {
	/// \brief ID of tile.
	TileId id{};
	/// \brief UV coordinates.
	kvf::UvRect uv{kvf::uv_rect_v};
};
} // namespace le
