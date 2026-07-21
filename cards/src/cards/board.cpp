#include "cards/board.hpp"
#include "cards/util.hpp"
#include "cards/world_space.hpp"
#include "le2d/input/listener_mapping.hpp"
#include <ranges>

namespace cards {
namespace {
constexpr auto next_location(Location const current) {
	switch (current) {
	case Location::South: return Location::East;
	case Location::North: return Location::West;
	default:
	case Location::West: return Location::South;
	case Location::East: return Location::North;
	}
}
} // namespace

Board::Board(gsl::not_null<IServices const*> services) : m_services(services) {
	create_hands();
	setup_sprites();
	bind_mapping();
}

void Board::draw(le::IRenderer& renderer) const {
	for (auto const& hand : std::views::reverse(m_hands)) { hand.draw(renderer); }
	m_trick.draw(renderer);
}

void Board::distribute(std::span<Denomination const> denominations) {
	if (denominations.empty()) { return; }

	while (!denominations.empty()) {
		auto face = Face::Up;
		for (auto& hand : m_hands) {
			hand.add_card(denominations.front(), face);
			face = Face::Down;
			denominations = denominations.subspan(1);
			if (denominations.empty()) { break; }
		}
	}
}

void Board::tick([[maybe_unused]] kvf::Seconds const dt) {
	//
}

void Board::create_hands() {
	for (auto const location : util::all_locations_v) {
		auto hand = Hand{&m_services->get_catalog(), location};
		hand.render_view.orientation = util::render_view_orientation(location);
		m_hands.push_back(std::move(hand));
	}
}

void Board::setup_sprites() {
	static constexpr auto bottom_size_v = glm::vec2{world_space_v.x, 0.25f * world_space_v.y};
	static constexpr auto bottom_origin_v = glm::vec2{0.0f, 0.5f * (-world_space_v.y + bottom_size_v.y)};

	static constexpr auto card_height_v = 0.19f * world_space_v.y;

	for (auto& hand : m_hands) {
		hand.set_y(bottom_origin_v.y);
		hand.set_card_height(card_height_v);
	};
}

void Board::bind_mapping() {
	auto mapping = std::make_shared<le::input::ListenerMapping>();
	mapping->on_cursor_pos = [this](le::event::CursorPos const& cursor_pos) {
		m_hands.front().on_cursor_move(m_services->get_unprojector().to_target(cursor_pos.normalized));
		return false;
	};
	mapping->on_mouse_button = [this](le::event::MouseButton const& button) {
		if (button.button != GLFW_MOUSE_BUTTON_LEFT || button.action != GLFW_RELEASE || button.mods != 0) { return false; }
		return m_hands.front().consume_mouse_click();
	};
	mapping->on_key = [this](le::event::Key const& key) {
		if (key.action != GLFW_PRESS || key.mods != 0) { return false; }
		switch (key.key) {
		case GLFW_KEY_ENTER: {
			if (auto const selected = m_hands.front().get_selected_card()) {
				auto card = *m_hands.front().remove_card(selected->get_denomination());
				static auto s_location = Location::South;
				card.location = s_location;
				s_location = next_location(s_location);
				m_trick.push_card(std::move(card));
			}
			break;
		}
		case GLFW_KEY_ESCAPE: {
			m_trick.cards.clear();
			break;
		}
		}
		return false;
	};
	m_mapping = std::move(mapping);
	m_services->get_input_router().push_mapping(m_mapping);
}
} // namespace cards
