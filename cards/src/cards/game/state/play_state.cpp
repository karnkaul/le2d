#include "cards/game/state/play_state.hpp"
#include <algorithm>

namespace cards::game {
auto PlayState::is_valid_submit(Denomination const denomination, std::span<Denomination const> hand) const -> bool {
	if (!trick_state.first || trump_suit == denomination.suit) { return true; }
	auto const running_suit = trick_state.get_running_suit();
	auto const has_running_suit = std::ranges::any_of(hand, [running_suit](Denomination const d) { return d.suit == running_suit; });
	return !has_running_suit || denomination.suit == running_suit;
}

auto PlayState::is_valid_submit(Denomination const denomination, std::span<Card const> hand) const -> bool {
	if (!trick_state.first || trump_suit == denomination.suit) { return true; }
	auto const running_suit = trick_state.get_running_suit();
	auto const has_running_suit = std::ranges::any_of(hand, [running_suit](Card const& c) { return c.get_denomination().suit == running_suit; });
	return !has_running_suit || denomination.suit == running_suit;
}
} // namespace cards::game
