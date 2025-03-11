#pragma once
#include <kvf/rect.hpp>
#include <cstdint>

namespace le {
enum struct TileId : std::int32_t { None = 0 }; // NOLINT(performance-enum-size)

struct Tile {
	TileId id{};
	kvf::UvRect uv{kvf::uv_rect_v};
};
} // namespace le
