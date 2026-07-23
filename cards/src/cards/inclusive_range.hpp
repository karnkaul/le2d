#pragma once
#include <algorithm>

namespace cards {
template <typename Type>
struct InclusiveRange {
	[[nodiscard]] constexpr auto in_range(Type const& in) const -> bool { return lo <= in && in <= hi; }
	[[nodiscard]] constexpr auto clamp(Type const& t) const -> Type { return std::clamp(t, lo, hi); }

	Type lo;
	Type hi;
};
} // namespace cards
