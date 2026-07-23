#pragma once
#include "cards/types.hpp"
#include "le2d/nvec2.hpp"
#include <array>
#include <bit>
#include <numeric>

namespace cards::util {
template <klib::EnumT E, std::size_t Count = std::size_t(E::Count_)>
constexpr auto enumerated_array() {
	auto ret = std::array<std::underlying_type_t<E>, Count>{};
	std::ranges::iota(ret, 0);
	return std::bit_cast<std::array<E, Count>>(ret);
};

inline constexpr auto all_suits_v = enumerated_array<Suit>();
inline constexpr auto all_values_v = enumerated_array<Value>();

inline constexpr auto all_denominations_count_v = std::size_t(Suit::Count_) * std::size_t(Value::Count_);
inline constexpr auto standard_denominations_v = [] {
	auto ret = std::array<Denomination, all_denominations_count_v>{};
	auto index = std::size_t{};
	for (auto const suit : all_suits_v) {
		for (auto const value : all_values_v) { ret.at(index++) = Denomination{.suit = suit, .value = value}; }
	}
	return ret;
}();

inline constexpr auto all_seats_v = enumerated_array<Seat>();

[[nodiscard]] constexpr auto next_seat(Seat const current) -> Seat { return Seat((int(current) + 1) % int(Seat::Count_)); }
[[nodiscard]] constexpr auto opposite_seat(Seat const seat) -> Seat { return Seat((int(seat) + 2) % int(Seat::Count_)); }

[[nodiscard]] constexpr auto seat_direction(Seat const seat) -> glm::vec2 {
	switch (seat) {
	default:
	case Seat::South: return -le::up_v;
	case Seat::East: return le::right_v;
	case Seat::North: return le::up_v;
	case Seat::West: return -le::right_v;
	}
}

[[nodiscard]] constexpr auto is_vertical(Seat const seat) -> bool { return seat == Seat::South || seat == Seat::North; }
[[nodiscard]] constexpr auto is_horizontal(Seat const seat) -> bool { return seat == Seat::East || seat == Seat::West; }

void shuffle(std::span<Denomination> denominations);

[[nodiscard]] auto render_view_orientation(Seat seat) -> le::nvec2;
} // namespace cards::util
