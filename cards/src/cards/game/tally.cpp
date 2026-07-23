#include "cards/game/tally.hpp"

namespace cards::game {
auto Tally::to_card_point(Value const value) const -> CardPoint {
	if (!custom_pointer) { return default_card_point(value); }
	return custom_pointer->to_card_point(value);
}

void Tally::triage(Round const& round) {
	auto round_points = 0;
	for (auto const& card : round.cards.values) { round_points += int(to_card_point(card->get_denomination().value)); }
	auto& target = points.at(round.winner);
	auto const total_points = int(target) + round_points;
	target = CardPoint{total_points};
}
} // namespace cards::game
