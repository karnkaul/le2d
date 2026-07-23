#pragma once
#include "cards/game/selector/selector.hpp"
#include "cards/inclusive_range.hpp"
#include "le2d/random.hpp"

namespace cards::game::selector {
class Timed : public Selector {
  public:
	explicit Timed(gsl::not_null<le::Random*> random, gsl::not_null<Hand*> hand, gsl::not_null<PlayState const*> play_state);

	auto select_submit() -> std::optional<Card> final;
	void tick(kvf::Seconds dt) override;

	InclusiveRange<kvf::Seconds> submit_interval{.lo = 0.2s, .hi = 1s};

  protected:
	// customize for intelligent selection.
	virtual auto timed_submit() -> Card { return submit_valid(); }

  private:
	void set_submit_remain();

	gsl::not_null<le::Random*> m_random;

	std::optional<kvf::Seconds> m_submit_remain{};
};
} // namespace cards::game::selector
