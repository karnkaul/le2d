#pragma once
#include "cards/game/card.hpp"
#include "cards/game/state/trick_state.hpp"
#include "cards/game/tally.hpp"

namespace cards::game {
struct PlayState {
	[[nodiscard]] auto is_valid_submit(Denomination denomination, std::span<Denomination const> hand) const -> bool;
	[[nodiscard]] auto is_valid_submit(Denomination denomination, std::span<Card const> hand) const -> bool;

	// current trick state.
	TrickState trick_state{};
	// game trump suit (if revealed/applicable).
	std::optional<Suit> trump_suit{};
	Tally tally{};
};
} // namespace cards::game
