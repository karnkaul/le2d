#include "cards/game/evaluator.hpp"
#include "cards/util.hpp"
#include "klib/debug/assert.hpp"

namespace cards::game {
auto Evaluator::to_rank(Value const value) const -> Rank {
	if (!custom_ranker) { return default_rank(value); }
	return custom_ranker->to_rank(value);
}

auto Evaluator::evaluate_winner(PlayState const& play_state) const -> Seat {
	auto const running_suit = play_state.trick_state.get_running_suit();
	KLIB_ASSERT(running_suit);

	auto const has_suit = [&](Suit const suit) {
		return std::ranges::any_of(play_state.trick_state.denominations.values, [&](auto const& denomination) {
			KLIB_ASSERT(denomination);
			return denomination->suit == suit;
		});
	};

	auto winning_suit = *running_suit;
	if (play_state.trump_suit && has_suit(*play_state.trump_suit)) { winning_suit = *play_state.trump_suit; }

	struct Candidate {
		Rank rank{};
		Seat seat{};
	};

	auto winner = Candidate{};
	for (auto const seat : util::all_seats_v) {
		auto const denomination = play_state.trick_state.denominations.at(seat);
		KLIB_ASSERT(denomination);
		if (denomination->suit != winning_suit) { continue; }
		auto const candidate = Candidate{.rank = to_rank(denomination->value), .seat = seat};
		if (candidate.rank < winner.rank) { continue; }
		winner = candidate;
	}

	KLIB_ASSERT(winner.rank > Rank{});
	return winner.seat;
}
} // namespace cards::game
