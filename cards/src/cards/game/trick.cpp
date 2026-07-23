#include "cards/game/trick.hpp"
#include "cards/util.hpp"
#include "klib/debug/assert.hpp"
#include <ranges>

namespace cards::game {
void Trick::draw(le::IRenderer& renderer) const {
	for (auto const seat : m_draw_order) {
		auto const& card = m_cards.at(seat);
		KLIB_ASSERT(card);
		auto render_view = le::Transform{};
		render_view.orientation = util::render_view_orientation(card->seat);
		renderer.set_view(render_view);
		card->draw(renderer);
	}
}

void Trick::push_card(Card card) {
	KLIB_ASSERT(!is_full());
	card.instance().transform.position = -0.25f * card.get_sprite().get_size();
	card.instance().tint = kvf::white_v;
	auto const seat = card.seat;
	m_state.denominations.at(seat) = card.get_denomination();
	m_cards.at(seat) = std::move(card);
	if (m_draw_order.empty()) { m_state.first = seat; }
	m_draw_order.push_back(seat);
}

auto Trick::transfer_round(Seat const winner) -> Round {
	KLIB_ASSERT(is_full());
	auto ret = Round{
		.cards = std::move(m_cards),
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
