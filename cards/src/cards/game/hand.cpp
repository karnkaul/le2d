#include "cards/game/hand.hpp"
#include "cards/util.hpp"
#include <algorithm>
#include <ranges>

namespace cards::game {
namespace {
constexpr auto lane_size_v = 0.25f * world_space_v;

[[nodiscard]] constexpr auto get_edge_offset(Seat const seat) -> glm::vec2 { return util::seat_direction(seat) * 0.5f * (world_space_v - lane_size_v); }
} // namespace

Hand::Hand(gsl::not_null<Catalog const*> catalog, Seat const seat) : m_catalog(catalog), m_seat(seat), m_datum(get_edge_offset(seat)) {}

void Hand::draw(le::IRenderer& renderer) const {
	for (auto const& card : m_cards) { card.draw(renderer); }
}

void Hand::add_card(Denomination const denomination, Face const face) {
	auto& card = m_cards.emplace_back(*m_catalog, denomination);
	card.set_face(face);
	on_cards_changed();
	synchronize();
}

auto Hand::remove_card(Denomination const denomination) -> std::optional<Card> {
	auto const it = std::ranges::find_if(m_cards, [denomination](Card const& card) { return card.get_denomination() == denomination; });
	if (it == m_cards.end()) { return {}; }

	auto ret = std::move(*it);
	m_cards.erase(it);
	on_cards_changed();
	synchronize();
	return ret;
}

void Hand::clear_cards() { m_cards.clear(); }

auto Hand::find_hovered(glm::vec2 const cursor_position) -> klib::Ptr<Card> {
	for (auto& card : std::views::reverse(m_cards)) {
		if (!card.get_sprite().bounding_rect().contains(cursor_position)) { continue; }
		return &card;
	}
	return {};
}

void Hand::synchronize() {
	if (util::is_vertical(m_seat)) {
		spread_horizontal();
	} else {
		spread_vertical();
	}
}

void Hand::spread_horizontal() {
	if (m_cards.empty()) { return; }

	KLIB_ASSERT(!m_cards.empty());

	auto const canvas_width = m_catalog->get_config().get_hand_canvas_size().x;
	auto const cell_width = canvas_width / float(m_cards.size());

	auto card_position = m_datum;
	card_position.x -= 0.5f * (canvas_width - cell_width);
	for (auto& card : m_cards) {
		auto& instance = card.instance();
		instance.tint = kvf::white_v;
		instance.transform.position = card_position;

		card_position.x += cell_width;
	}
}

void Hand::spread_vertical() {
	if (m_cards.empty()) { return; }

	KLIB_ASSERT(!m_cards.empty());

	auto const canvas_height = m_catalog->get_config().get_hand_canvas_size().y;
	auto const cell_height = canvas_height / float(m_cards.size());

	auto card_position = m_datum;
	card_position.y += 0.5f * (canvas_height - cell_height);
	for (auto& card : m_cards) {
		auto& instance = card.instance();
		instance.tint = kvf::white_v;
		instance.transform.position = card_position;

		card_position.y -= cell_height;
	}
}

PlayerHand::PlayerHand(gsl::not_null<Catalog const*> catalog) : Hand(catalog, seat_v) {}

auto PlayerHand::set_hovered(glm::vec2 const cursor_position) -> klib::Ptr<Card const> {
	m_hovered = find_hovered(cursor_position);
	synchronize();
	return {};
}

void PlayerHand::on_cards_changed() { m_hovered = {}; }

void PlayerHand::synchronize() {
	Hand::synchronize();

	if (m_hovered) {
		auto const dy = m_catalog->get_config().get_select_offset();
		m_hovered->instance().transform.position.y += dy;
	}
}
} // namespace cards::game
