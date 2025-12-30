#pragma once
#include <kvf/time.hpp>
#include <cstdint>

namespace le {
struct FrameStats {
	kvf::Seconds total_dt{};
	kvf::Seconds frame_dt{};
	kvf::Seconds run_time{};
	std::int32_t framerate{};
	std::uint64_t total_frames{};
};
} // namespace le
