#pragma once
#include <cstdint>
#include <span>

namespace le::embedded {
[[nodiscard]] auto spirv_vert() -> std::span<std::uint32_t const>;
[[nodiscard]] auto spirv_frag() -> std::span<std::uint32_t const>;
} // namespace le::embedded
