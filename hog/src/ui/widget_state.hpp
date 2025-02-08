#pragma once
#include <cstdint>

namespace hog::ui {
enum class WidgetState : std::uint8_t { None, Hover, Press, COUNT_ };
} // namespace hog::ui
