#include "cards/game/selector/timed.hpp"

namespace cards::game::selector {
Timed::Timed(gsl::not_null<le::Random*> random, gsl::not_null<Hand*> hand, gsl::not_null<PlayState const*> play_state)
	: Selector(hand, play_state), m_random(random) {}

auto Timed::select_submit() -> std::optional<Card> {
	if (m_hand->get_denominations().empty()) { return {}; }

	if (!m_submit_remain) {
		set_submit_remain();
		return {};
	}

	if (*m_submit_remain > 0s) { return {}; }

	m_submit_remain.reset();
	return timed_submit();
}

void Timed::tick(kvf::Seconds const dt) {
	if (m_submit_remain && m_submit_remain > 0s) { *m_submit_remain -= dt; }
}

void Timed::set_submit_remain() { m_submit_remain = kvf::Seconds{m_random->next_float(submit_interval.lo.count(), submit_interval.hi.count())}; }
} // namespace cards::game::selector
