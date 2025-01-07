#pragma once
#include <kvf/bitmap.hpp>
#include <kvf/color.hpp>
#include <kvf/render_device.hpp>
#include <bit>

namespace le {
inline constexpr auto color_format_v{vk::Format::eR8G8B8A8Srgb};

inline constexpr auto white_pixel_bytes_v = std::bit_cast<std::array<std::byte, sizeof(kvf::Color)>>(kvf::white_v);
inline constexpr auto white_bitmap_v = kvf::Bitmap{.bytes = white_pixel_bytes_v, .size = {1, 1}};
} // namespace le
