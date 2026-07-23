#pragma once
#include "cards/catalog/catalog.hpp"
#include "cards/game/hand.hpp"
#include "cards/game/per_seat.hpp"
#include "cards/game/trick.hpp"
#include "le2d/drawable/shape.hpp"

namespace cards::game {
class Board : public le::IDrawable {
  public:
	static constexpr auto player_seat_v{Seat::South};

	explicit Board(gsl::not_null<Catalog const*> catalog);

	void draw(le::IRenderer& renderer) const final;

	void distribute(std::span<Denomination const> denominations);

	[[nodiscard]] auto& get_hand(this auto&& self, Seat const seat) { return self.m_hands.at(seat).value(); }
	[[nodiscard]] auto& get_player_hand(this auto&& self) { return self.get_hand(player_seat_v); }

	game::Trick trick{};
	std::vector<Card> discarded{};

	std::optional<Seat> current_seat{};

  private:
	PerSeat<std::optional<Hand>> m_hands{};
	le::drawable::Triangle m_indicator{};
};
} // namespace cards::game
