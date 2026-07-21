#pragma once
#include "cards/hand.hpp"
#include "cards/services.hpp"
#include "cards/trick.hpp"

namespace cards {
class Board : public le::IDrawable {
  public:
	explicit Board(gsl::not_null<IServices const*> services);

	void draw(le::IRenderer& renderer) const final;

	void distribute(std::span<Denomination const> denominations);

	void tick(kvf::Seconds dt);

  private:
	void create_hands();
	void setup_sprites();
	void bind_mapping();

	gsl::not_null<IServices const*> m_services;

	std::vector<Hand> m_hands{};
	Trick m_trick{};

	std::shared_ptr<le::input::IMapping> m_mapping{};
};
} // namespace cards
