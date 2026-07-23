#pragma once
#include "cards/game/card.hpp"

namespace cards::game {
class Hand : public le::IDrawable {
  public:
	explicit Hand(gsl::not_null<Catalog const*> catalog, Seat seat);

	void draw(le::IRenderer& renderer) const final;

	[[nodiscard]] auto get_seat() const -> Seat { return m_seat; }
	[[nodiscard]] auto get_cards() const -> std::span<Card const> { return m_cards; }

	void add_card(Denomination denomination, Face face = Face::Up);
	auto remove_card(Denomination denomination) -> std::optional<Card>;
	void clear_cards();

  protected:
	virtual void on_cards_changed() {}
	virtual void synchronize();

	[[nodiscard]] auto find_hovered(glm::vec2 cursor_position) -> klib::Ptr<Card>;

	gsl::not_null<Catalog const*> m_catalog;

  private:
	void spread_horizontal();
	void spread_vertical();

	Seat m_seat;

	glm::vec2 m_datum{};

	std::vector<Card> m_cards{};
};

class PlayerHand : public Hand {
  public:
	static constexpr auto seat_v{Seat::South};

	explicit PlayerHand(gsl::not_null<Catalog const*> catalog);

	[[nodiscard]] auto get_hovered() const -> klib::Ptr<Card const> { return m_hovered; }
	auto set_hovered(glm::vec2 cursor_position) -> klib::Ptr<Card const>;

  private:
	void on_cards_changed() final;
	void synchronize() final;

	klib::Ptr<Card> m_hovered{};
};
} // namespace cards::game
