#pragma once
#include "klib/enum/map.hpp"
#include <cstdint>

namespace cards {
enum class Suit : std::int8_t {
	Hearts,
	Spades,
	Diamonds,
	Clubs,
	Count_,
};
inline auto const suit_name_map = klib::EnumNameMap<Suit>{
	{Suit::Hearts, "hearts"},
	{Suit::Spades, "spades"},
	{Suit::Diamonds, "diamonds"},
	{Suit::Clubs, "clubs"},
};

enum class Value : std::int8_t {
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
	Count_,
};
inline auto const value_name_map = klib::EnumNameMap<Value>{
	{Value::Ace, "ace"},   {Value::Two, "two"},		{Value::Three, "three"}, {Value::Four, "four"}, {Value::Five, "five"},
	{Value::Six, "six"},   {Value::Seven, "seven"}, {Value::Eight, "eight"}, {Value::Nine, "nine"}, {Value::Ten, "ten"},
	{Value::Jack, "jack"}, {Value::Queen, "queen"}, {Value::King, "king"},
};

enum class Face : std::int8_t { Up, Down };
inline auto const face_name_map = klib::EnumNameMap<Face>{
	{Face::Up, "up"},
	{Face::Down, "down"},
};

struct Denomination {
	auto operator==(Denomination const&) const -> bool = default;

	Suit suit{Suit::Hearts};
	Value value{Value::Ace};
};

enum class Seat : std::int8_t { South, North, West, East, Count_ };
inline auto const seat_name_map = klib::EnumNameMap<Seat>{
	{Seat::South, "south"},
	{Seat::North, "north"},
	{Seat::West, "west"},
	{Seat::East, "east"},
};

// can be weakly ordered: accumulates over rounds.
enum struct CardPoint : int {};
[[nodiscard]] constexpr auto default_card_point(Value const value) -> CardPoint {
	static_assert(Value::Ace == Value{0});
	if (value == Value::Ace) { return CardPoint{int(Value::King) + 2}; }
	return CardPoint{int(value) + 1};
}

// must be strongly ordered: determines round winner.
enum struct Rank : int {};
[[nodiscard]] constexpr auto default_rank(Value const value) -> Rank { return static_cast<Rank>(default_card_point(value)); }
} // namespace cards
