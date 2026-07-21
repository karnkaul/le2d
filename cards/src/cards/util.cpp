#include "cards/util.hpp"
#include <algorithm>
#include <random>

namespace cards {
void util::shuffle(std::span<Denomination> denominations) {
	static auto s_engine = std::default_random_engine{std::random_device{}()};
	std::ranges::shuffle(denominations, s_engine);
}

auto util::render_view_orientation(Location const location) -> le::nvec2 {
	switch (location) {
	default:
	case Location::South: return le::nvec2{};
	case Location::North: return le::nvec2{}.rotated(glm::radians(180.0f));
	case Location::West: return le::nvec2{}.rotated(glm::radians(90.0f));
	case Location::East: return le::nvec2{}.rotated(glm::radians(270.0f));
	}
}
} // namespace cards
