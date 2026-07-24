#pragma once
#include "cards/catalog/catalog.hpp"
#include "cards/game/pawn.hpp"
#include "cards/game/per_seat.hpp"
#include "cards/game/trick.hpp"
#include "le2d/drawable/shape.hpp"

namespace cards::game {
class Board : public le::IDrawable {
  public:
	static constexpr auto player_seat_v{Seat::South};

	explicit Board(gsl::not_null<Catalog const*> catalog, Pawn::Builder const& pawn_builder);

	void draw(le::IRenderer& renderer) const final;

	void tick(kvf::Seconds dt);

	[[nodiscard]] auto are_hands_empty() const -> bool;

	[[nodiscard]] auto& get_pawn(this auto&& self, Seat const seat) { return self.m_pawns.at(seat).value(); }

	auto select_submit() -> std::optional<Card>;

	game::Trick trick{};
	std::vector<Card> discarded{};
	PlayState play_state{};

	std::optional<Seat> current_seat{};

  private:
	void create_pawns(Pawn::Builder const& builder);

	void draw_indicator(le::IRenderer& renderer) const;

	gsl::not_null<Catalog const*> m_catalog;

	PerSeat<std::optional<Pawn>> m_pawns{};
	le::drawable::Triangle m_indicator{};
};
} // namespace cards::game
