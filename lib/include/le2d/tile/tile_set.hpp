#pragma once
#include <le2d/tile/tile.hpp>
#include <vector>

namespace le {
class TileSet {
  public:
	[[nodiscard]] auto get_tiles() const -> std::span<Tile const> { return m_sorted_tiles; }
	void set_tiles(std::vector<Tile> tiles);

	[[nodiscard]] auto get_uv(TileId id) const -> kvf::UvRect;

	[[nodiscard]] auto is_loaded() const -> bool { return !m_sorted_tiles.empty(); }

  private:
	std::vector<Tile> m_sorted_tiles{};
};
} // namespace le
