#include <drawer.hpp>
#include <le2d/drawable/sprite.hpp>

namespace cards {
namespace {
constexpr auto to_tile_id(Card const card) { return le::TileId((int(card.suit) * 13) + int(card.rank)); }
} // namespace

void Drawer::draw(le::Renderer& renderer, Card const card, glm::vec2 const position) const {
	if (!deck_sheet) { return; }
	auto const tile_id = to_tile_id(card);
	auto sprite = le::drawable::Sprite{};
	sprite.set_base_size(glm::vec2{card_height});
	sprite.set_resize_aspect(kvf::ResizeAspect::FixHeight);
	sprite.set_tile(deck_sheet, tile_id);
	sprite.transform.position = position;
	sprite.draw(renderer);
}
} // namespace cards
