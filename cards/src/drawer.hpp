#pragma once
#include <card.hpp>
#include <le2d/texture.hpp>

namespace le {
class Renderer;
} // namespace le

namespace cards {
struct Drawer {
	le::TileSheet const* deck_sheet{};
	float card_height{200.0f};

	void draw(le::Renderer& renderer, Card card, glm::vec2 position) const;
};
} // namespace cards
