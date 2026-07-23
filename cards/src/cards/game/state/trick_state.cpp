#include "cards/game/state/trick_state.hpp"
#include "klib/debug/assert.hpp"

namespace cards::game {
auto TrickState::get_running_suit() const -> std::optional<Suit> {
	if (!first) { return {}; }
	auto const first_denomination = denominations.at(*first);
	KLIB_ASSERT(first_denomination.has_value());
	return first_denomination->suit;
}
} // namespace cards::game
