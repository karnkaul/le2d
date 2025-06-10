#pragma once
#include <kvf/color.hpp>
#include <le2d/transform.hpp>

namespace le {
/// \brief Instance data for instanced rendering.
struct RenderInstance {
	Transform transform{};
	kvf::Color tint{kvf::white_v};
};
} // namespace le
