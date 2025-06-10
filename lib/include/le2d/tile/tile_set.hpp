#pragma once
#include <le2d/tile/tile.hpp>
#include <vector>

namespace le {
/// \brief Sorted set of Tiles.
class TileSet {
  public:
	[[nodiscard]] auto get_tiles() const -> std::span<Tile const> { return m_sorted_tiles; }
	/// \param tiles Tiles that will be stored and sorted.
	void set_tiles(std::vector<Tile> tiles);

	/// \brief Get the UV coordinates for a given Tile ID.
	/// \param id Tile ID to query.
	/// \returns UV rect for tile if found, else uv_rect_v.
	[[nodiscard]] auto get_uv(TileId id) const -> kvf::UvRect;

	[[nodiscard]] auto is_loaded() const -> bool { return !m_sorted_tiles.empty(); }

  private:
	std::vector<Tile> m_sorted_tiles{};
};
} // namespace le
