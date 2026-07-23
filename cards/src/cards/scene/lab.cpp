#include "cards/scene/lab.hpp"
#include "cards/game/evaluator.hpp"
#include "cards/game/selector/player.hpp"
#include "cards/game/selector/timed.hpp"
#include "cards/util.hpp"
#include "klib/string/fixed_string.hpp"
#include "le2d/input/listener_mapping.hpp"
#include <imgui.h>
#include <array>

namespace cards::scene {
void Lab::initialize() {
	bind_mapping();
	m_board.emplace(&get_coordinator().get_catalog());

	clear_color = kvf::Color{0x333333ff};

	for (auto const seat : util::all_seats_v) {
		if (seat == Seat::South) {
			m_selectors.at(seat) = std::make_unique<game::selector::Player>(&get_coordinator(), &m_board->get_player_hand(), &m_play_state);
			continue;
		}
		m_selectors.at(seat) = std::make_unique<game::selector::Timed>(&get_coordinator().get_random(), &m_board->get_hand(seat), &m_play_state);
	}

	rebuild_deck();
}

void Lab::tick([[maybe_unused]] kvf::Seconds const dt) {
	if (m_board->get_player_hand().get_denominations().empty()) { rebuild_deck(); }

	try_submit_next();

	for (auto& selector : m_selectors.values) { selector->tick(dt); }

	try_complete_round(dt);

	m_play_state.end_frame();

	if (ImGui::Begin("Debug")) {
		auto const seat_name = m_board->current_seat ? seat_name_map.to_name(*m_board->current_seat) : "[none]";
		ImGui::TextUnformatted(klib::FixedString{"current seat: {}", seat_name}.c_str());
		if (ImGui::TreeNode("tally")) {
			for (auto const seat : util::all_seats_v) {
				auto const points = m_tally.points.at(seat);
				ImGui::TextUnformatted(klib::FixedString{"{}: {}", seat_name_map.to_name(seat), int(points)}.c_str());
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void Lab::render(le::IRenderer& renderer) const { m_board->draw(renderer); }

void Lab::bind_mapping() {
	auto mapping = std::make_shared<le::input::ListenerMapping>();

	mapping->on_key = [this](le::event::Key const& key) {
		if (key.action != GLFW_PRESS || key.mods != 0) { return false; }
		switch (key.key) {
		case GLFW_KEY_ENTER: m_play_state.submit_selected = m_board->current_seat == Seat::South; break;
		}
		return false;
	};

	m_mapping = std::move(mapping);
	get_coordinator().get_input_router().push_mapping(m_mapping);
}

void Lab::rebuild_deck() {
	static constexpr auto to_remove_v = std::array{
		Value::Two, Value::Three, Value::Four, Value::Five, Value::Six,
	};
	static constexpr auto should_remove_v = [](Denomination const d) { return std::ranges::find(to_remove_v, d.value) != to_remove_v.end(); };
	auto denominations = std::vector<Denomination>{};
	denominations.append_range(util::standard_denominations_v);
	std::erase_if(denominations, should_remove_v);
	util::shuffle(denominations);

	m_board->distribute(denominations);
	m_board->trick.clear_round();
	m_board->discarded.clear();
	m_board->current_seat = Seat::South;
	m_play_state = {};
	m_tally = {};
}

void Lab::submit_and_advance(Card card) {
	card.set_face(Face::Up);
	m_board->trick.push_card(std::move(card));
	m_board->current_seat = util::next_seat(*m_board->current_seat);
	m_play_state.trick_state = m_board->trick.get_state();
}

void Lab::try_submit_next() {
	if (!m_board->current_seat) { return; }

	auto card = m_selectors.at(*m_board->current_seat)->select_submit();
	if (!card) { return; }

	submit_and_advance(std::move(*card));
}

void Lab::try_complete_round(kvf::Seconds const dt) {
	if (!m_board->trick.is_full()) {
		m_complete_round_remain = m_complete_round_interval;
		return;
	}

	m_board->current_seat.reset();
	if (m_complete_round_remain > 0s) {
		m_complete_round_remain -= dt;
		return;
	}

	auto const winner = game::Evaluator{}.evaluate_winner(m_play_state);
	auto round = m_board->trick.transfer_round(winner);
	m_tally.triage(round);
	for (auto& card : round.cards.values) { m_board->discarded.push_back(std::move(card.value())); }

	m_board->current_seat = winner;
	m_play_state.trick_state = {};
}
} // namespace cards::scene
