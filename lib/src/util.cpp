#include <klib/env.hpp>
#include <kvf/is_positive.hpp>
#include <le2d/util.hpp>

namespace le {
auto util::exe_path() -> std::string const& { return klib::env::exe_path(); }

auto util::divide_into_tiles(int const rows, int const cols) -> std::vector<Tile> {
	if (rows <= 0 || cols <= 0) { return {}; }
	auto const tile_size = glm::vec2{1.0f / float(cols), 1.0f / float(rows)};
	auto ret = std::vector<Tile>{};
	ret.reserve(std::size_t(rows * cols));
	auto lt = glm::vec2{};
	auto id = std::to_underlying(TileId{1});
	for (auto row = 0; row < rows; ++row) {
		for (auto col = 0; col < cols; ++col) {
			auto const tile = Tile{.id = TileId{id++}, .uv = kvf::UvRect{.lt = lt, .rb = lt + tile_size}};
			ret.push_back(tile);
			lt.x += tile_size.x;
		}
		lt.x = 0;
		lt.y += tile_size.y;
	}
	return ret;
}

auto util::generate_flipbook_timeline(std::span<Tile const> tiles, kvf::Seconds duration) -> anim::Timeline<TileId> {
	if (tiles.empty() || duration <= 0s) { return {}; }
	auto ret = anim::Timeline<TileId>{.duration = duration};
	ret.keyframes.reserve(tiles.size());
	auto const tile_time = duration / tiles.size();
	auto timestamp = kvf::Seconds{0s};
	for (auto const& tile : tiles) {
		ret.keyframes.push_back(anim::Keyframe<TileId>{.timestamp = timestamp, .payload = tile.id});
		timestamp += tile_time;
	}
	return ret;
}
} // namespace le
