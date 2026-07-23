#pragma once
#include "cards/game/card.hpp"

namespace cards::game {
class Hand : public le::IDrawable {
  public:
	explicit Hand(gsl::not_null<Catalog const*> catalog, Seat seat);

	void draw(le::IRenderer& renderer) const final;

	[[nodiscard]] auto get_seat() const -> Seat { return m_seat; }

	[[nodiscard]] auto get_denominations() const -> std::span<Denomination const> { return m_denominations; }

	[[nodiscard]] auto at_cursor(glm::vec2 world_position) const -> std::optional<Denomination>;

	[[nodiscard]] auto get_selected() const -> std::optional<Denomination>;
	void select_card(Denomination denomination);
	void unselect_card();

	void add_card(Denomination denomination, Face face = Face::Up);
	auto remove_card(Denomination denomination) -> std::optional<Card>;
	void clear_cards();

  private:
	void synchronize();

	gsl::not_null<Catalog const*> m_catalog;
	Seat m_seat;
	float m_n_canvas_width;
	float m_card_height{200.0f};
	le::Transform m_render_view{};

	std::vector<Card> m_cards{};
	std::vector<Denomination> m_denominations{};

	klib::Ptr<Card> m_selected{};
};
} // namespace cards::game
