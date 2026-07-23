#pragma once
#include "klib/enum/map.hpp"
#include "kvf/time.hpp"

namespace cards {
enum class TimingType : std::int8_t { Slow, Normal, Fast };
inline auto const timing_type_name_map = klib::EnumNameMap<TimingType>{
	{TimingType::Slow, "slow"},
	{TimingType::Normal, "normal"},
	{TimingType::Fast, "fast"},
};

struct Timings {
	kvf::Seconds deal_rate{0.02s};
	kvf::Seconds submit_ttl{0.2s};
	kvf::Seconds discard_delay{2s};
	kvf::Seconds discard_ttl{0.2s};
};

namespace timing {
inline constexpr auto slow_v = Timings{
	.deal_rate = 0.05s,
	.submit_ttl = 0.5s,
	.discard_delay = 3s,
	.discard_ttl = 0.5s,
};

inline constexpr auto normal_v = Timings{
	.deal_rate = 0.02s,
	.submit_ttl = 0.2s,
	.discard_delay = 2s,
	.discard_ttl = 0.2s,
};

inline constexpr auto fast_v = Timings{
	.deal_rate = 0.01s,
	.submit_ttl = 0.1s,
	.discard_delay = 1s,
	.discard_ttl = 0.1s,
};

} // namespace timing

[[nodiscard]] constexpr auto to_timings(TimingType const type) -> Timings {
	switch (type) {
	case TimingType::Slow: return timing::slow_v;
	case TimingType::Fast: return timing::fast_v;
	default: return timing::normal_v;
	}
}
} // namespace cards
