#include <kvf/is_positive.hpp>
#include <le2d/util.hpp>
#include <array>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <climits>
#endif

namespace le {
auto util::exe_path() -> std::string {
	static auto ret = std::string{}; // can never change throughout the process existance
	if (!ret.empty()) { return ret; }

#if defined(_WIN32)
	auto buffer = std::array<char, MAX_PATH>{};
	DWORD length = GetModuleFileNameA(nullptr, buffer.data(), buffer.size());
	if (length == 0) { return {}; }
	ret = std::string{buffer.data(), length};
#elif defined(__linux__)
	auto buffer = std::array<char, PATH_MAX>{};
	ssize_t length = ::readlink("/proc/self/exe", buffer.data(), buffer.size());
	if (length == -1) { return {}; }
	ret = std::string{buffer.data(), std::size_t(length)};
#endif

	return ret;
}

auto util::divide_into_tiles(glm::vec2 const size, int const rows, int const cols) -> std::vector<Tile> {
	if (!kvf::is_positive(size) || rows <= 0 || cols <= 0) { return {}; }
	auto const tile_size = glm::vec2{size.x / float(cols), size.y / float(rows)};
	auto ret = std::vector<Tile>{};
	ret.reserve(std::size_t(rows * cols));
	auto const size_inv = 1.0f / size;
	auto lt = glm::vec2{};
	auto id = std::to_underlying(TileId{1});
	for (auto row = 0; row < rows; ++row) {
		for (auto col = 0; col < cols; ++col) {
			auto tile = Tile{.id = TileId{id++}, .uv = kvf::UvRect{.lt = lt, .rb = lt + tile_size}};
			tile.uv.lt *= size_inv;
			tile.uv.rb *= size_inv;
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
