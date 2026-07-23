#pragma once
#include "cards/game/hand.hpp"
#include "cards/game/selector/selector.hpp"
#include "cards/services.hpp"
#include "le2d/input/mapping.hpp"

namespace cards::game::selector {
class Player : public Selector {
  public:
	explicit Player(gsl::not_null<IServices const*> services, gsl::not_null<Hand*> hand, gsl::not_null<PlayState const*> play_state);

	auto select_submit() -> std::optional<Card> final;
	void tick(kvf::Seconds dt) final;

  private:
	void bind_mapping();

	auto consume_mouse_click() -> bool;

	gsl::not_null<IServices const*> m_services;
	std::shared_ptr<le::input::IMapping> m_mapping{};

	std::optional<Denomination> m_hovered{};
};
} // namespace cards::game::selector
