#pragma once
#include <cstdint>

namespace le {
struct RenderStats {
	std::int64_t draw_calls{};
	std::int64_t triangles{};

	constexpr void accumulate(RenderStats const& other) {
		draw_calls += other.draw_calls;
		triangles += other.triangles;
	}

	[[nodiscard]] constexpr auto accumulated(RenderStats const& other) const -> RenderStats {
		auto ret = *this;
		ret.accumulate(other);
		return ret;
	}
};
} // namespace le
