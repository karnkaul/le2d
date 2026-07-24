#include "cards/game/trick.hpp"
#include "cards/util.hpp"
#include "klib/debug/assert.hpp"
#include <algorithm>

namespace cards::game {
void Trick::draw(le::IRenderer& renderer) const {
	for (auto const seat : m_draw_order) {
		auto const& card = m_cards.at(seat);
		KLIB_ASSERT(card);
		card->draw(renderer);
	}
}

auto Trick::get_target_position(Card const& card, Seat const seat) -> glm::vec2 { return 0.3f * card.get_sprite().get_size() * util::seat_direction(seat); }

auto Trick::is_full() const -> bool {
	return std::ranges::all_of(m_cards.values, [](std::optional<Card> const& card) { return card.has_value(); });
}

void Trick::push_card(Card card, Seat const seat) {
	KLIB_ASSERT(!is_full());
	card.instance().transform.position = get_target_position(card, seat);
	m_state.denominations.at(seat) = card.get_denomination();
	m_cards.at(seat) = std::move(card);
	if (m_draw_order.empty()) { m_state.first = seat; }
	m_draw_order.push_back(seat);
}

auto Trick::transfer_round(Seat const winner) -> Round {
	KLIB_ASSERT(is_full());
	auto ret = Round{
		.cards =
			{
				.values =
					{
						std::move(*m_cards.values[0]),
						std::move(*m_cards.values[1]),
						std::move(*m_cards.values[2]),
						std::move(*m_cards.values[3]),
					},
			},
		.first = m_draw_order.front(),
		.winner = winner,
	};
	clear_round();
	return ret;
}

void Trick::clear_round() {
	m_cards = {};
	m_state = {};
	m_draw_order.clear();
}
} // namespace cards::game
