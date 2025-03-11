#pragma once
#include <klib/enum_array.hpp>
#include <cstdint>
#include <optional>
#include <string_view>

namespace le {
enum class Vsync : std::int8_t { Strict, Adaptive, MultiBuffer, Off, COUNT_ };

inline constexpr auto vsync_str_v = klib::EnumArray<Vsync, std::string_view>{"strict", "adaptive", "multi", "off"};

[[nodiscard]] constexpr auto parse_vsync(std::string_view const in) -> std::optional<Vsync> {
	if (in.size() == 1 && in.front() >= '0') {
		auto const ret = Vsync(in.front() - '0');
		if (ret < Vsync::COUNT_) { return ret; }
	}
	for (auto v = Vsync{}; v < Vsync::COUNT_; v = Vsync(int(v) + 1)) {
		if (vsync_str_v[v] == in) { return v; }
	}
	return {};
}
} // namespace le
