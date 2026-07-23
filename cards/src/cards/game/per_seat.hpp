#pragma once
#include "cards/types.hpp"
#include <array>

namespace cards::game {
template <typename ValueT>
class PerSeat {
  public:
	static constexpr auto count_v = std::size_t(Seat::Count_);

	[[nodiscard]] auto& at(this auto&& self, Seat const seat) { return self.values.at(std::size_t(seat)); }

	std::array<ValueT, count_v> values{};
};
} // namespace cards::game
