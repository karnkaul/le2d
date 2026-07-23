#pragma once
#include "cards/game/per_seat.hpp"
#include "cards/types.hpp"
#include <optional>

namespace cards::game {
struct TrickState {
	[[nodiscard]] auto get_running_suit() const -> std::optional<Suit>;

	PerSeat<std::optional<Denomination>> denominations{};
	std::optional<Seat> first{};
};
} // namespace cards::game
