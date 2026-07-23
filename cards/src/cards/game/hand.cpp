#include "cards/game/hand.hpp"
#include "cards/util.hpp"
#include <algorithm>
#include <ranges>

namespace cards::game {
namespace {
constexpr auto bottom_height_v = 0.25f * world_space_v.y;
constexpr auto bottom_y_v = 0.5f * (-world_space_v.y + bottom_height_v);
} // namespace

Hand::Hand(gsl::not_null<Catalog const*> catalog, Seat const seat)
	: m_catalog(catalog), m_seat(seat), m_n_canvas_width(catalog->get_config().hand_n_canvas_width), m_card_height(catalog->get_config().get_card_height()) {
	m_render_view.orientation = util::render_view_orientation(seat);
}

void Hand::draw(le::IRenderer& renderer) const {
	renderer.set_view(m_render_view);
	for (auto const& card : m_cards) { card.draw(renderer); }
}

void Hand::add_card(Denomination const denomination, Face const face) {
	auto& card = m_cards.emplace_back(*m_catalog, denomination);
	card.set_face(face);
	card.set_height(m_card_height);
	card.seat = m_seat;
	m_selected = {};
	synchronize();
}

auto Hand::remove_card(Denomination const denomination) -> std::optional<Card> {
	auto const it = std::ranges::find_if(m_cards, [denomination](Card const& card) { return card.get_denomination() == denomination; });
	if (it == m_cards.end()) { return {}; }

	auto ret = std::move(*it);
	m_cards.erase(it);
	m_selected = {};
	synchronize();
	return ret;
}

void Hand::clear_cards() { m_cards.clear(); }

auto Hand::at_cursor(glm::vec2 const world_position) const -> std::optional<Denomination> {
	for (auto const& card : std::views::reverse(m_cards)) {
		if (!card.get_sprite().bounding_rect().contains(world_position)) { continue; }
		return card.get_denomination();
	}
	return {};
}

auto Hand::get_selected() const -> std::optional<Denomination> {
	if (!m_selected) { return {}; }
	return m_selected->get_denomination();
}

void Hand::select_card(Denomination const denomination) {
	auto const it = std::ranges::find_if(m_cards, [denomination](Card const& card) { return card.get_denomination() == denomination; });
	if (it == m_cards.end()) { return; }

	m_selected = &*it;
	synchronize();
}

void Hand::unselect_card() {
	m_selected = {};
	synchronize();
}

void Hand::synchronize() {
	m_denominations.clear();
	if (m_cards.empty()) { return; }

	m_denominations.reserve(m_cards.size());

	auto const canvas_width = m_n_canvas_width * world_space_v.x;
	auto const cell_width = canvas_width / float(m_cards.size());

	auto card_position = glm::vec2{0.0f, bottom_y_v};
	card_position.x -= 0.5f * (canvas_width - cell_width);
	for (auto& card : m_cards) {

		auto& instance = card.instance();
		instance.tint = kvf::white_v;
		instance.transform.position = card_position;

		card_position.x += cell_width;

		m_denominations.push_back(card.get_denomination());
	}

	if (m_selected) {
		auto const dy = m_catalog->get_config().n_spacing.y * m_catalog->get_config().get_card_height();
		m_selected->instance().transform.position.y += dy;
	}
}
} // namespace cards::game
