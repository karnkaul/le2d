#pragma once
#include <cstdint>

namespace hog::ui {
enum class WidgetState : std::uint8_t { None, Hover, Press, Disabled, COUNT_ };
} // namespace hog::ui
