#include "cards/game/selector/selector.hpp"
#include "klib/debug/assert.hpp"

namespace cards::game {
Selector::Selector(gsl::not_null<Hand*> hand, gsl::not_null<PlayState const*> play_state) : m_hand(hand), m_play_state(play_state) {}

auto Selector::select_submit() -> std::optional<Card> {
	if (m_hand->get_cards().empty()) { return {}; }
	return submit_valid();
}

auto Selector::submit_valid() -> Card {
	auto const cards = m_hand->get_cards();
	KLIB_ASSERT(!cards.empty());

	if (auto const running_suit = m_play_state->trick_state.get_running_suit()) {
		auto const it = std::ranges::find_if(cards, [running_suit](Card const& c) { return c.get_denomination().suit == *running_suit; });
		if (it != cards.end()) { return remove_and_submit(it->get_denomination()); }
	}

	return remove_and_submit(cards.front().get_denomination());
}

auto Selector::remove_and_submit(Denomination const denomination) -> Card {
	KLIB_ASSERT(m_play_state->is_valid_submit(denomination, m_hand->get_cards()));
	auto ret = m_hand->remove_card(denomination);
	KLIB_ASSERT(ret);
	return std::move(*ret);
}
} // namespace cards::game
