#include "cards/game/pawn.hpp"
#include "cards/game/selector/player_selector.hpp"

namespace cards::game {
Pawn::Builder::Builder(gsl::not_null<IServices const*> services) : m_services(services) {}

auto Pawn::Builder::build_for(Seat const seat, gsl::not_null<PlayState const*> play_state) const -> Pawn {
	auto hand = std::make_unique<Hand>(&m_services->get_catalog(), seat);
	auto selector = std::make_unique<Selector>(hand.get(), play_state);
	return Pawn{std::move(hand), std::move(selector)};
}

auto Pawn::Builder::build_player(gsl::not_null<PlayState const*> play_state) const -> Pawn {
	auto hand = std::make_unique<PlayerHand>(&m_services->get_catalog());
	auto selector = std::make_unique<PlayerSelector>(m_services, hand.get(), play_state);
	return Pawn{std::move(hand), std::move(selector)};
}
} // namespace cards::game
