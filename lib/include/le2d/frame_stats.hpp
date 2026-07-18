#pragma once
#include "kvf/time.hpp"
#include <cstdint>

namespace le {
enum struct Framerate : std::int32_t {};

struct FrameStats {
	kvf::Seconds total_dt{};
	kvf::Seconds frame_dt{};
	kvf::Seconds run_time{};
	Framerate framerate{};
	std::uint64_t total_frames{};
};
} // namespace le
