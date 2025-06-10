#pragma once
#include <cstdint>

namespace le {
/// \brief Strongly typed integer for text height.
enum struct TextHeight : std::uint8_t {
	Min = 8,
	Default = 40,
	Max = 200,
};
} // namespace le
