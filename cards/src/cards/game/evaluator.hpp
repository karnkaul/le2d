#pragma once
#include "cards/game/state/play_state.hpp"
#include "klib/base_types.hpp"

namespace cards::game {
class Evaluator : public klib::Polymorphic {
  public:
	class IRanker;

	[[nodiscard]] auto to_rank(Value value) const -> Rank;

	[[nodiscard]] auto evaluate_winner(PlayState const& play_state) const -> Seat;

	klib::Ptr<IRanker const> custom_ranker{};
};

class Evaluator::IRanker : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto to_rank(Value value) const -> Rank = 0;
};
} // namespace cards::game
