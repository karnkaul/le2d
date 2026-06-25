#pragma once
#include "klib/enum/name.hpp"
#include <cstdint>

namespace le {
enum class Vsync : std::int8_t { Strict, Adaptive, MultiBuffer, Off, COUNT_ };

inline auto const vsync_name_map = klib::EnumNameMap<Vsync>{
	{Vsync::Strict, "strict"},
	{Vsync::Adaptive, "adaptive"},
	{Vsync::MultiBuffer, "multi"},
	{Vsync::Off, "off"},
};
} // namespace le
