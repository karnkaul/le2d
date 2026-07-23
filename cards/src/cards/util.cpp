#include "cards/util.hpp"
#include <algorithm>
#include <random>

namespace cards {
namespace {
static_assert(util::next_seat(Seat::South) == Seat::East);
static_assert(util::next_seat(Seat::East) == Seat::North);
static_assert(util::next_seat(Seat::North) == Seat::West);
static_assert(util::next_seat(Seat::West) == Seat::South);

static_assert(util::opposite_seat(Seat::North) == Seat::South);
static_assert(util::opposite_seat(Seat::South) == Seat::North);
static_assert(util::opposite_seat(Seat::East) == Seat::West);
static_assert(util::opposite_seat(Seat::West) == Seat::East);
} // namespace

void util::shuffle(std::span<Denomination> denominations) {
	static auto s_engine = std::default_random_engine{std::random_device{}()};
	std::ranges::shuffle(denominations, s_engine);
}

auto util::render_view_orientation(Seat const seat) -> le::nvec2 {
	switch (seat) {
	default:
	case Seat::South: return le::nvec2{};
	case Seat::East: return le::nvec2{}.rotated(glm::radians(270.0f));
	case Seat::North: return le::nvec2{}.rotated(glm::radians(180.0f));
	case Seat::West: return le::nvec2{}.rotated(glm::radians(90.0f));
	}
}
} // namespace cards
