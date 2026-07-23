#include "cards/game/board.hpp"
#include "cards/util.hpp"
#include <ranges>

namespace cards::game {
Board::Board(gsl::not_null<Catalog const*> catalog) {
	for (auto const seat : util::all_seats_v) {
		if (seat == PlayerHand::seat_v) {
			auto player_hand = std::make_unique<PlayerHand>(catalog);
			m_player_hand = player_hand.get();
			m_hands.at(seat) = std::move(player_hand);
			continue;
		}

		m_hands.at(seat) = std::make_unique<Hand>(catalog, seat);
	}

	m_indicator.instance.transform.position.y = -0.2f * world_space_v.y;
	m_indicator.geometry.create(30.0f);
	m_indicator.instance.transform.orientation.rotate(glm::radians(180.0f));
}

void Board::draw(le::IRenderer& renderer) const {
	draw_indicator(renderer);
	renderer.set_view({});
	for (auto const& hand : std::views::reverse(m_hands.values)) { hand->draw(renderer); }
	trick.draw(renderer);
}

void Board::distribute(std::span<Denomination const> denominations) {
	if (denominations.empty()) { return; }

	while (!denominations.empty()) {
		auto face = Face::Up;
		for (auto& hand : m_hands.values) {
			hand->add_card(denominations.front(), face);
			face = Face::Down;
			denominations = denominations.subspan(1);
			if (denominations.empty()) { break; }
		}
	}
}

void Board::draw_indicator(le::IRenderer& renderer) const {
	if (!current_seat) { return; }

	auto render_view = le::Transform{};
	render_view.orientation = util::render_view_orientation(*current_seat);
	renderer.set_view(render_view);
	m_indicator.draw(renderer);
}
} // namespace cards::game
