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

inline constexpr auto all_locations_v = enumerated_array<Location>();

void shuffle(std::span<Denomination> denominations);

[[nodiscard]] auto render_view_orientation(Location location) -> le::nvec2;
} // namespace cards::util
