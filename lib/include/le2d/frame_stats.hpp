#pragma once
#include <kvf/time.hpp>
#include <cstdint>

namespace le {
struct FrameStats {
	kvf::Seconds total_dt{};
	kvf::Seconds present_dt{};
	std::int32_t framerate{};
};
} // namespace le
