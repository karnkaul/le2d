#pragma once
#include <cstdint>
#include <span>

namespace le::spirv {
[[nodiscard]] auto vert() -> std::span<std::uint32_t const>;
[[nodiscard]] auto frag() -> std::span<std::uint32_t const>;
} // namespace le::spirv
