#include "cards/game/selector/player.hpp"
#include "le2d/input/listener_mapping.hpp"

namespace cards::game {
PlayerSelector::PlayerSelector(gsl::not_null<IServices const*> services, gsl::not_null<PlayerHand*> player_hand, gsl::not_null<PlayState const*> play_state)
	: Selector(player_hand, play_state), m_services(services), m_player_hand(player_hand) {
	bind_mapping();
}

auto PlayerSelector::select_submit() -> std::optional<Card> {
	if (!m_submit) { return {}; }

	auto const hovered = m_player_hand->get_hovered();
	if (!hovered) { return {}; }

	if (!m_play_state->is_valid_submit(hovered->get_denomination(), m_player_hand->get_cards())) { return {}; }

	return remove_and_submit(hovered->get_denomination());
}

void PlayerSelector::tick([[maybe_unused]] kvf::Seconds const dt) { m_submit = false; }

void PlayerSelector::bind_mapping() {
	auto mapping = std::make_shared<le::input::ListenerMapping>();

	mapping->on_cursor_pos = [this](le::event::CursorPos const& cursor_pos) {
		auto const world_position = m_services->get_unprojector().to_target(cursor_pos.normalized);
		m_player_hand->set_hovered(world_position);
		return false;
	};

	mapping->on_mouse_button = [this](le::event::MouseButton const& button) {
		if (button.button != GLFW_MOUSE_BUTTON_LEFT || button.action != GLFW_RELEASE || button.mods != 0) { return false; }
		return consume_mouse_click();
	};

	m_mapping = std::move(mapping);
	m_services->get_input_router().push_mapping(m_mapping);
}

auto PlayerSelector::consume_mouse_click() -> bool {
	if (!m_player_hand->get_hovered()) { return false; }
	m_submit = true;
	return true;
}
} // namespace cards::game
