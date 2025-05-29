#pragma once
#include <glm/vec2.hpp>
#include <klib/enum_array.hpp>
#include <cstdint>
#include <string_view>

namespace cards {
enum class Suit : std::int8_t { Hearts, Spades, Diamonds, Clubs, COUNT_ };

inline constexpr auto suit_str_v = klib::EnumArray<Suit, std::string_view>{"H", "S", "D", "C"};

enum class Rank : std::int8_t {
	None,
	Ace,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Ten,
	Jack,
	Queen,
	King,
	COUNT_,
};

inline constexpr auto rank_str_v = klib::EnumArray<Rank, std::string_view>{
	"n/a", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K",
};

struct Card {
	Suit suit{Suit::Hearts};
	Rank rank{Rank::Ace};

	auto operator==(Card const&) const -> bool = default;
};
} // namespace cards
