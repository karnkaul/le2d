#pragma once
#include "cards/game/state/trick_state.hpp"

namespace cards::game {
struct PlayState {
	[[nodiscard]] auto is_valid_submit(Denomination denomination, std::span<Denomination const> hand) const -> bool;

	void end_frame();

	// current trick state.
	TrickState trick_state{};
	// game trump suit (if revealed/applicable).
	std::optional<Suit> trump_suit{};

	// set if PlayerSelector should submit the selected Card (if any).
	// resets every frame.
	bool submit_selected{};
};
} // namespace cards::game
