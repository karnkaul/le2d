#include "cards/trick.hpp"
#include "cards/util.hpp"

namespace cards {
void Trick::draw(le::IRenderer& renderer) const {
	for (auto const& card : cards) {
		auto render_view = le::Transform{};
		render_view.orientation = util::render_view_orientation(card.location);
		renderer.set_view(render_view);
		card.draw(renderer);
	}
}

void Trick::push_card(CardSprite card) {
	card.instance().transform.position = -0.25f * card.get_sprite().get_size();
	card.instance().tint = kvf::white_v;
	cards.push_back(std::move(card));
}
} // namespace cards
