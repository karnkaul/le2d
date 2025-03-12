#include <le2d/tile/tile_set.hpp>
#include <algorithm>

namespace le {
void TileSet::set_tiles(std::vector<Tile> tiles) {
	m_sorted_tiles = std::move(tiles);
	std::ranges::sort(m_sorted_tiles, [](Tile const& a, Tile const& b) { return a.id < b.id; });
}

auto TileSet::get_uv(TileId const id) const -> kvf::UvRect {
	if (id == TileId::None) { return kvf::uv_rect_v; }
	static constexpr auto proj = [](Tile const& t) { return t.id; };
	auto const [first, _] = std::ranges::equal_range(m_sorted_tiles, id, {}, proj);
	if (first == m_sorted_tiles.end()) { return kvf::uv_rect_v; }
	return first->uv;
}
} // namespace le
