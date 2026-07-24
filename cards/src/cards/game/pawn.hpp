#pragma once
#include "cards/game/hand.hpp"
#include "cards/game/selector/selector.hpp"
#include "cards/services.hpp"

namespace cards::game {
class Pawn : public le::IDrawable {
  public:
	class Builder;

	explicit Pawn(std::unique_ptr<Hand> hand, std::unique_ptr<Selector> selector) : m_hand(std::move(hand)), m_selector(std::move(selector)) {}

	void draw(le::IRenderer& renderer) const final { m_hand->draw(renderer); }

	void tick(kvf::Seconds const dt) { m_selector->tick(dt); }

	[[nodiscard]] auto select_submit() const -> std::optional<Card> { return m_selector->select_submit(); }

	void add_card(Denomination denomination, Face const face = Face::Up) { m_hand->add_card(denomination, face); }

	[[nodiscard]] auto is_hand_empty() const -> bool { return m_hand->get_cards().empty(); }

  private:
	std::unique_ptr<Hand> m_hand{};
	std::unique_ptr<Selector> m_selector{};
};

class Pawn::Builder : public klib::Polymorphic {
  public:
	explicit Builder(gsl::not_null<IServices const*> services);

	[[nodiscard]] virtual auto build_for(Seat seat, gsl::not_null<PlayState const*> play_state) const -> Pawn;
	[[nodiscard]] auto build_player(gsl::not_null<PlayState const*> play_state) const -> Pawn;

  private:
	gsl::not_null<IServices const*> m_services;
};

class PlayerPawnBuilder : public Pawn::Builder {
  public:
	explicit PlayerPawnBuilder(gsl::not_null<IServices const*> services) : Builder(services) {}

	[[nodiscard]] auto build_for(Seat const seat, gsl::not_null<PlayState const*> play_state) const -> Pawn final {
		if (seat == PlayerHand::seat_v) { return build_player(play_state); }
		return Builder::build_for(seat, play_state);
	}
};
} // namespace cards::game
