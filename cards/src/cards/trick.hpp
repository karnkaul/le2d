#pragma once
#include "cards/card_sprite.hpp"

namespace cards {
class Trick : public le::IDrawable {
  public:
	void draw(le::IRenderer& renderer) const final;

	void push_card(CardSprite card);

	std::vector<CardSprite> cards{};
};
} // namespace cards
