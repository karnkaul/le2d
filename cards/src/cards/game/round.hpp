#pragma once
#include "cards/game/card.hpp"
#include "cards/game/per_seat.hpp"

namespace cards::game {
struct Round {
	PerSeat<std::optional<Card>> cards{};
	Seat first{};
	Seat winner{};
};
} // namespace cards::game
