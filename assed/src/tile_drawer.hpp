#pragma once
#include "le2d/drawable/shape.hpp"
#include <optional>

namespace le::assed {
class TileDrawer {
  public:
	explicit TileDrawer() { clear(); }

	[[nodiscard]] auto create_tile_frame(kvf::Rect<> const& rect) const -> drawable::LineRect;
	void setup(std::span<Tile const> tiles, glm::vec2 texture_size);

	void clear();

	void update();
	void draw(IRenderer& renderer) const;

	void inspect_style();

	drawable::Quad quad{};
	std::vector<drawable::LineRect> tile_frames{};
	std::optional<std::size_t> selected_tile{};

	struct {
		kvf::Color frame_color{kvf::white_v};
		kvf::Color selected_color{kvf::green_v};
		float frame_width{2.0f};
	} style{};
};
} // namespace le::assed
