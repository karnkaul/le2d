#include "cards/scene/lab.hpp"
#include "cards/util.hpp"
#include "le2d/input/listener_mapping.hpp"
#include <imgui.h>
#include <array>

namespace cards::scene {
namespace {
struct Ranker : game::Evaluator::IRanker {
	[[nodiscard]] auto to_rank(Value value) const -> Rank final {
		switch (value) {
		case Value::Jack: return default_rank(Value::Ace);
		case Value::Nine: return default_rank(Value::King);
		case Value::Ace: return default_rank(Value::Queen);
		case Value::Ten: return default_rank(Value::Jack);
		case Value::King: return default_rank(Value::Ten);
		case Value::Queen: return default_rank(Value::Nine);
		default: return default_rank(value);
		}
	}
};

struct Pointer : game::Tally::IPointer {
	[[nodiscard]] auto to_card_point(Value value) const -> CardPoint final {
		switch (value) {
		case Value::Jack: return CardPoint{3};
		case Value::Nine: return CardPoint{2};
		case Value::Ace: return CardPoint{1};
		case Value::Ten: return CardPoint{1};
		default: return CardPoint{0};
		}
	}
};

auto const g_ranker = Ranker{};
auto const g_pointer = Pointer{};
} // namespace

void Lab::initialize() {
	bind_mapping();

	clear_color = kvf::Color{0x114422ff};
	m_conductor.emplace(&get_coordinator());
	rebuild_deck();
}

void Lab::tick(kvf::Seconds const dt) {
	m_conductor->tick(dt);
	if (m_conductor->get_state() == game::Conductor::State::Idle) { rebuild_deck(); }
}

void Lab::render(le::IRenderer& renderer) const { m_conductor->draw(renderer); }

void Lab::bind_mapping() {
	auto mapping = std::make_shared<le::input::ListenerMapping>();

	mapping->on_key = [this](le::event::Key const& key) {
		if (key.action != GLFW_PRESS || key.mods != 0) { return false; }
		switch (key.key) {
		case GLFW_KEY_ESCAPE: get_coordinator().get_context().set_window_should_close(); break;
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

	auto const game_info = game::GameInfo{
		.custom_deck = denominations,
		.custom_ranker = &g_ranker,
		.custom_pointer = &g_pointer,
	};
	m_conductor->start_game(game_info);
}
} // namespace cards::scene
