#pragma once
#include "cards/game/card.hpp"
#include "cards/game/per_seat.hpp"
#include "cards/game/round.hpp"
#include "cards/game/state/trick_state.hpp"
#include <optional>

namespace cards::game {
class Trick : public le::IDrawable {
  public:
	using State = TrickState;

	void draw(le::IRenderer& renderer) const final;

	[[nodiscard]] static auto get_target_position(Card const& card, Seat seat) -> glm::vec2;

	[[nodiscard]] auto is_full() const -> bool;

	void push_card(Card card, Seat seat);
	[[nodiscard]] auto get_state() const -> State const& { return m_state; }
	[[nodiscard]] auto transfer_round(Seat winner) -> Round;
	void clear_round();

  private:
	PerSeat<std::optional<Card>> m_cards{};
	State m_state{};
	std::vector<Seat> m_draw_order{};
};
} // namespace cards::game
