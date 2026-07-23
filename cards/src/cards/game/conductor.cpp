#include "cards/game/conductor.hpp"
#include "cards/game/selector/player.hpp"
#include "cards/util.hpp"
#include "klib/string/fixed_string.hpp"
#include "klib/visitor.hpp"
#include <imgui.h>

namespace cards::game {
Conductor::Conductor(gsl::not_null<IServices const*> services)
	: m_services(services), m_timings(&services->get_catalog().get_config().timings), m_board(&services->get_catalog()) {
	create_selectors();
}

void Conductor::draw(le::IRenderer& renderer) const {
	m_board.draw(renderer);
	auto const visitor = klib::SubVisitor{
		[&](Submit const& submit) { submit.draw(renderer); },
		[&](Discard const& discard) { discard.draw(renderer); },
	};
	std::visit(visitor, m_anim);
}

void Conductor::create_selectors() {
	for (auto const seat : util::all_seats_v) {
		if (seat == Seat::South) {
			m_selectors.at(seat) = std::make_unique<game::PlayerSelector>(m_services, &m_board.get_player_hand(), &m_play_state);
			continue;
		}
		m_selectors.at(seat) = std::make_unique<game::Selector>(&m_board.get_hand(seat), &m_play_state);
	}
}

void Conductor::tick(kvf::Seconds const dt) {
	switch (m_state) {
	default:
	case State::Idle: break;
	case State::Dealing: tick_deal(dt); break;
	case State::WaitingForSubmit: tick_wait_submit(dt); break;
	case State::Submitting: tick_submit(dt); break;
	case State::Discarding: tick_discard(dt); break;
	}
	shared_tick(dt);
}

void Conductor::start_game(GameInfo const& game_info) {
	KLIB_ASSERT(m_state == State::Idle);

	m_game_info = game_info;
	auto denominations = std::vector<Denomination>{};
	denominations.append_range(m_game_info.custom_deck);
	if (denominations.empty()) { denominations.append_range(util::standard_denominations_v); }
	util::shuffle(denominations);

	m_anim = Deal{.denominations = std::move(denominations), .remain = m_timings->deal_rate};

	m_board.trick.clear_round();
	m_board.discarded.clear();
	m_board.current_seat.reset();

	m_play_state = {};
	m_play_state.tally.custom_pointer = m_game_info.custom_pointer;

	m_state = State::Dealing;
}

void Conductor::tick_deal(kvf::Seconds const dt) {
	auto& deal = std::get<Deal>(m_anim);

	deal.remain -= dt;
	if (deal.remain > 0s) { return; }

	auto const face = deal.next_seat == PlayerHand::seat_v ? Face::Up : Face::Down;
	m_board.get_hand(deal.next_seat).add_card(deal.denominations.back(), face);
	deal.denominations.pop_back();
	if (deal.denominations.empty()) {
		finish_deal();
		return;
	}

	deal.remain = m_timings->deal_rate;
	deal.next_seat = util::next_seat(deal.next_seat);
}

void Conductor::tick_wait_submit([[maybe_unused]] kvf::Seconds dt) {
	auto card = m_selectors.at(*m_board.current_seat)->select_submit();
	if (!card) { return; }

	start_submit(std::move(*card));
}

void Conductor::tick_submit(kvf::Seconds const dt) {
	auto& submit = std::get<Submit>(m_anim);

	submit.elapsed += dt;
	auto const alpha = submit.elapsed / m_timings->submit_ttl;
	if (alpha > 1.0f) {
		finish_submit();
		return;
	}

	submit.card.instance().transform.position = glm::mix(submit.src, submit.dst, alpha);
}

void Conductor::tick_discard(kvf::Seconds const dt) {
	auto& m_discard = std::get<Discard>(m_anim);

	m_discard.start_remain -= dt;
	if (m_discard.start_remain > 0s) { return; }

	m_discard.elapsed += dt;
	auto const alpha = m_discard.elapsed / m_timings->discard_ttl;
	if (alpha > 1.0f) {
		finish_discard();
		return;
	}

	m_discard.position = glm::mix(glm::vec2{}, m_discard.dst, alpha);
	for (auto const seat : util::all_seats_v) {
		auto& card = m_discard.round.cards.at(seat);
		card->instance().transform.position = m_discard.position + m_discard.offsets.at(seat);
	}
}

void Conductor::shared_tick(kvf::Seconds const dt) {
	for (auto& selector : m_selectors.values) { selector->tick(dt); }

	ImGui::SetNextWindowSize({200.0f, 200.0f}, ImGuiCond_Once);
	if (ImGui::Begin("Conductor")) {
		ImGui::TextUnformatted(klib::FixedString{"state: {}", state_name_map.to_name(m_state)}.c_str());
		auto const seat_name = m_board.current_seat ? seat_name_map.to_name(*m_board.current_seat) : "[none]";
		ImGui::TextUnformatted(klib::FixedString{"current seat: {}", seat_name}.c_str());
		auto const running_suit = m_board.trick.get_state().get_running_suit();
		auto const running_suit_name = running_suit ? suit_name_map.to_name(*running_suit) : "[none]";
		ImGui::TextUnformatted(klib::FixedString{"running suit: {}", running_suit_name}.c_str());
		if (ImGui::TreeNode("tally")) {
			for (auto const seat : util::all_seats_v) {
				auto const points = m_play_state.tally.points.at(seat);
				ImGui::TextUnformatted(klib::FixedString{"{}: {}", seat_name_map.to_name(seat), int(points)}.c_str());
			}
			ImGui::TreePop();
		}
		auto const* discard = std::get_if<Discard>(&m_anim);
		auto const winner_name = discard ? seat_name_map.to_name(discard->round.winner) : "[none]";
		ImGui::TextUnformatted(klib::FixedString{"winner: {}", winner_name}.c_str());
	}
	ImGui::End();
}

void Conductor::finish_deal() {
	m_anim = std::monostate{};
	// TODO: trump selection, dealer
	m_board.current_seat = util::all_seats_v.at(m_services->get_random().next_index(util::all_seats_v.size()));
	m_state = State::WaitingForSubmit;
}

void Conductor::start_submit(Card card) {
	KLIB_ASSERT(m_board.current_seat);
	card.set_face(Face::Up);
	auto const src = card.instance().transform.position;
	auto const dst = Trick::get_target_position(card, *m_board.current_seat);
	m_anim = Submit{
		.seat = *m_board.current_seat,
		.card = std::move(card),
		.src = src,
		.dst = dst,
	};
	m_state = State::Submitting;
}

void Conductor::finish_submit() {
	auto& submit = std::get<Submit>(m_anim);
	submit.card.instance().transform.position = submit.dst;

	m_board.trick.push_card(std::move(submit.card), submit.seat);
	m_play_state.trick_state = m_board.trick.get_state();

	m_anim = std::monostate{};

	if (m_board.trick.is_full()) {
		start_discard();
		return;
	}

	m_board.current_seat = util::next_seat(*m_board.current_seat);
	m_state = State::WaitingForSubmit;
}

void Conductor::start_discard() {
	auto evaluator = Evaluator{};
	evaluator.custom_ranker = m_game_info.custom_ranker;
	auto const winner = evaluator.evaluate_winner(m_play_state);
	auto discard = Discard{.round = m_board.trick.transfer_round(winner)};
	auto const card_size = discard.round.cards.values.front()->get_sprite().get_size();
	auto const target_space = 0.5f * (world_space_v + (2.0f * card_size));
	discard.dst = util::seat_direction(winner) * target_space;
	discard.start_remain = m_timings->discard_delay;
	m_play_state.tally.triage(discard.round);
	for (auto const seat : util::all_seats_v) { discard.offsets.at(seat) = discard.round.cards.at(seat)->instance().transform.position; }
	m_anim = std::move(discard);

	m_board.current_seat.reset();
	m_state = State::Discarding;
}

void Conductor::finish_discard() {
	auto& discard = std::get<Discard>(m_anim);
	for (auto& card : discard.round.cards.values) { m_board.discarded.push_back(std::move(card.value())); }
	m_board.current_seat = discard.round.winner;
	m_play_state.trick_state = {};
	m_anim = std::monostate{};

	if (m_board.get_player_hand().get_cards().empty()) {
		m_state = State::Idle;
		return;
	}

	m_state = State::WaitingForSubmit;
}

void Conductor::Submit::draw(le::IRenderer& renderer) const { card.draw(renderer); }

void Conductor::Discard::draw(le::IRenderer& renderer) const {
	auto seat = round.first;
	for (auto i = 0; i < 4; ++i) {
		auto const& card = round.cards.at(seat);
		KLIB_ASSERT(card);
		card->draw(renderer);
		seat = util::next_seat(seat);
	}
}
} // namespace cards::game
