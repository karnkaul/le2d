#include "cards/scene/lab.hpp"
#include "cards/util.hpp"
#include <array>

namespace cards::scene {
void Lab::initialize() {
	build_deck();
	clear_color = kvf::Color{0x333333ff};
}

void Lab::tick(kvf::Seconds const dt) { m_board->tick(dt); }

void Lab::render(le::IRenderer& renderer) const { m_board->draw(renderer); }

void Lab::build_deck() {
	static constexpr auto to_remove_v = std::array{
		Value::Two, Value::Three, Value::Four, Value::Five, Value::Six,
	};
	static constexpr auto should_remove_v = [](Denomination const d) { return std::ranges::find(to_remove_v, d.value) != to_remove_v.end(); };
	auto cards = std::vector<Denomination>{};
	cards.append_range(util::standard_denominations_v);
	std::erase_if(cards, should_remove_v);
	util::shuffle(cards);

	m_board.emplace(&get_coordinator());
	m_board->distribute(cards);
}
} // namespace cards::scene
