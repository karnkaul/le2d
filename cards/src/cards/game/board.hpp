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

	[[nodiscard]] auto get_player_hand() const -> PlayerHand& { return *m_player_hand; }

	[[nodiscard]] auto get_hand(Seat const seat) const -> Hand& { return *m_hands.at(seat); }

	game::Trick trick{};
	std::vector<Card> discarded{};

	std::optional<Seat> current_seat{};

  private:
	void draw_indicator(le::IRenderer& renderer) const;

	PerSeat<std::unique_ptr<Hand>> m_hands{};
	klib::Ptr<PlayerHand> m_player_hand{};
	le::drawable::Triangle m_indicator{};
};
} // namespace cards::game
