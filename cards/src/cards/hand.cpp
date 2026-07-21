#include "cards/hand.hpp"
#include "cards/world_space.hpp"
#include <algorithm>
#include <ranges>

namespace cards {
namespace {
[[nodiscard]] auto find_hovered(std::span<CardSprite> cards, glm::vec2 const cursor) -> klib::Ptr<CardSprite> {
	for (auto& card : std::views::reverse(cards)) {
		if (!card.get_sprite().bounding_rect().contains(cursor)) { continue; }
		return &card;
	}
	return nullptr;
}
} // namespace

Hand::Hand(gsl::not_null<Catalog const*> catalog, Location const location) : m_catalog(catalog), m_location(location) {}

void Hand::draw(le::IRenderer& renderer) const {
	renderer.set_view(render_view);
	for (auto const& card : m_cards) { card.draw(renderer); }
}

void Hand::add_card(Denomination const denomination, Face const face) {
	m_selected = m_hovered = {};
	auto& card = m_cards.emplace_back(*m_catalog, denomination);
	card.set_face(face);
	card.location = m_location;
	update_sprites();
}

auto Hand::remove_card(Denomination const denomination) -> std::optional<CardSprite> {
	auto const it = std::ranges::find_if(m_cards, [denomination](CardSprite const& card) { return card.get_denomination() == denomination; });
	if (it == m_cards.end()) { return {}; }

	m_selected = m_hovered = {};
	auto ret = std::move(*it);
	m_cards.erase(it);
	update_sprites();
	return ret;
}

void Hand::clear_cards() {
	m_selected = m_hovered = {};
	m_cards.clear();
}

void Hand::set_card_height(float const height) {
	m_card_height = height;
	update_sprites();
}

void Hand::set_y(float const y) {
	m_y = y;
	update_sprites();
}

void Hand::on_cursor_move(glm::vec2 const world_position) {
	m_hovered = find_hovered(m_cards, world_position);
	update_sprites();
}

auto Hand::consume_mouse_click() -> bool {
	if (!m_hovered) { return false; }

	if (m_selected && m_selected == m_hovered) {
		m_selected = {};
	} else {
		m_selected = m_hovered;
	}
	update_sprites();
	return true;
}

void Hand::update_sprites() {
	if (m_cards.empty()) { return; }

	for (auto& card : m_cards) { card.set_height(m_card_height); }
	auto card_size = m_cards.front().get_sprite().get_size();

	auto const canvas_width = world_space_v.x - (3.2f * (card_size.y));
	auto const cell_width = canvas_width / float(m_cards.size());

	auto card_position = glm::vec2{0.0f, m_y};
	card_position.x -= 0.5f * (canvas_width - cell_width);
	for (auto& card : m_cards) {

		auto& instance = card.instance();
		instance.tint = kvf::white_v;
		instance.transform.position = card_position;

		card_position.x += cell_width;
	}

	if (m_hovered) { m_hovered->instance().tint = kvf::yellow_v; }
	if (m_selected) {
		auto const padding = m_catalog->get_config().n_spacing * card_size;
		m_selected->instance().transform.position.y += padding.y;
	}
}
} // namespace cards
