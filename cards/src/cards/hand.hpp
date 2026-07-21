#pragma once
#include "cards/card_sprite.hpp"

namespace cards {
class Hand : public le::IDrawable {
  public:
	explicit Hand(gsl::not_null<Catalog const*> catalog, Location location);

	void draw(le::IRenderer& renderer) const final;

	[[nodiscard]] auto get_location() const -> Location { return m_location; }

	void add_card(Denomination denomination, Face face = Face::Up);
	auto remove_card(Denomination denomination) -> std::optional<CardSprite>;
	void clear_cards();

	[[nodiscard]] auto get_card_height() const -> float { return m_card_height; }
	void set_card_height(float height);

	[[nodiscard]] auto get_y() const -> float { return m_y; }
	void set_y(float y);

	[[nodiscard]] auto get_cards() const -> std::span<CardSprite const> { return m_cards; }
	[[nodiscard]] auto get_cards() -> std::span<CardSprite> { return m_cards; }

	void on_cursor_move(glm::vec2 world_position);
	[[nodiscard]] auto consume_mouse_click() -> bool;

	[[nodiscard]] auto get_hovered_card() const -> klib::Ptr<CardSprite const> { return m_hovered; }
	[[nodiscard]] auto get_selected_card() const -> klib::Ptr<CardSprite const> { return m_selected; }

	le::Transform render_view{};

  private:
	void update_sprites();

	gsl::not_null<Catalog const*> m_catalog;
	Location m_location;

	std::vector<CardSprite> m_cards{};

	float m_card_height{200.0f};
	float m_y{};

	klib::Ptr<CardSprite> m_hovered{};
	klib::Ptr<CardSprite> m_selected{};
};
} // namespace cards
