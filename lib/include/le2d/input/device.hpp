#pragma once
#include <cstdint>

namespace le::input {
/// \brief Input Device enumeration.
enum class Device : std::int8_t { Keyboard, Mouse, Gamepad };
} // namespace le::input
