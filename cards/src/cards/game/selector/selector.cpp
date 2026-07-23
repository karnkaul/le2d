#include "cards/game/selector/selector.hpp"
#include "klib/debug/assert.hpp"

namespace cards::game {
Selector::Selector(gsl::not_null<Hand*> hand, gsl::not_null<PlayState const*> play_state) : m_hand(hand), m_play_state(play_state) {}

auto Selector::select_submit() -> std::optional<Card> {
	if (m_hand->get_denominations().empty()) { return {}; }
	return submit_valid();
}

auto Selector::submit_valid() -> Card {
	auto const denominations = m_hand->get_denominations();
	KLIB_ASSERT(!denominations.empty());

	if (auto const running_suit = m_play_state->trick_state.get_running_suit()) {
		auto const it = std::ranges::find_if(denominations, [running_suit](Denomination const d) { return d.suit == *running_suit; });
		if (it != denominations.end()) { return remove_and_submit(*it); }
	}

	return remove_and_submit(denominations.front());
}

auto Selector::remove_and_submit(Denomination const denomination) -> Card {
	KLIB_ASSERT(m_play_state->is_valid_submit(denomination, m_hand->get_denominations()));
	auto ret = m_hand->remove_card(denomination);
	KLIB_ASSERT(ret);
	return std::move(*ret);
}
} // namespace cards::game
