#include "cards/game/selector/player.hpp"
#include "le2d/input/listener_mapping.hpp"

namespace cards::game::selector {
Player::Player(gsl::not_null<IServices const*> services, gsl::not_null<Hand*> hand, gsl::not_null<PlayState const*> play_state)
	: Selector(hand, play_state), m_services(services) {
	bind_mapping();
}

auto Player::select_submit() -> std::optional<Card> {
	if (!m_play_state->submit_selected) { return {}; }

	auto const selected = m_hand->get_selected();
	if (!selected) { return {}; }

	if (!m_play_state->is_valid_submit(*selected, m_hand->get_denominations())) { return {}; }

	return remove_and_submit(*selected);
}

void Player::tick([[maybe_unused]] kvf::Seconds const dt) {}

void Player::bind_mapping() {
	auto mapping = std::make_shared<le::input::ListenerMapping>();

	mapping->on_cursor_pos = [this](le::event::CursorPos const& cursor_pos) {
		auto const world_position = m_services->get_unprojector().to_target(cursor_pos.normalized);
		m_hovered = m_hand->at_cursor(world_position);
		return false;
	};

	mapping->on_mouse_button = [this](le::event::MouseButton const& button) {
		if (button.button != GLFW_MOUSE_BUTTON_LEFT || button.action != GLFW_RELEASE || button.mods != 0) { return false; }
		return consume_mouse_click();
	};

	m_mapping = std::move(mapping);
	m_services->get_input_router().push_mapping(m_mapping);
}

auto Player::consume_mouse_click() -> bool {
	if (!m_hovered) { return false; }

	auto selected = m_hand->get_selected();
	if (selected && selected == m_hovered) {
		m_hand->unselect_card();
	} else {
		m_hand->select_card(*m_hovered);
	}

	return true;
}
} // namespace cards::game::selector
