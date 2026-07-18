#pragma once
#include "klib/enum/map.hpp"
#include <cstdint>

namespace le::input {
/// \brief Input Device enumeration.
enum class Device : std::int8_t { Keyboard, Mouse, Gamepad };
inline auto const device_name_map = klib::EnumNameMap<Device>{
	{Device::Keyboard, "keyboard"},
	{Device::Mouse, "mouse"},
	{Device::Gamepad, "gamepad"},
};
} // namespace le::input
