#pragma once
#include "cards/game/hand.hpp"
#include "cards/game/selector/selector.hpp"
#include "cards/services.hpp"
#include "le2d/input/mapping.hpp"

namespace cards::game {
class PlayerSelector : public Selector {
  public:
	explicit PlayerSelector(gsl::not_null<IServices const*> services, gsl::not_null<PlayerHand*> player_hand, gsl::not_null<PlayState const*> play_state);

	auto select_submit() -> std::optional<Card> final;
	void tick(kvf::Seconds dt) final;

  private:
	void bind_mapping();

	auto consume_mouse_click() -> bool;

	gsl::not_null<IServices const*> m_services;
	gsl::not_null<PlayerHand*> m_player_hand;

	std::shared_ptr<le::input::IMapping> m_mapping{};

	bool m_submit{};
};
} // namespace cards::game
