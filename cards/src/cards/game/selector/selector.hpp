#pragma once
#include "cards/game/hand.hpp"
#include "cards/game/state/play_state.hpp"
#include "klib/base_types.hpp"
#include "kvf/time.hpp"

namespace cards::game {
class Selector : public klib::Polymorphic {
  public:
	explicit Selector(gsl::not_null<Hand*> hand, gsl::not_null<PlayState const*> play_state);

	virtual auto select_submit() -> std::optional<Card>;
	virtual void tick([[maybe_unused]] kvf::Seconds dt) {}

  protected:
	[[nodiscard]] auto submit_valid() -> Card;
	// denomination must be a valid submit, and in current hand.
	[[nodiscard]] auto remove_and_submit(Denomination denomination) -> Card;

	gsl::not_null<Hand*> m_hand;
	gsl::not_null<PlayState const*> m_play_state;
};
} // namespace cards::game
