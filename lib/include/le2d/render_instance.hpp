#pragma once
#include "le2d/transform.hpp"
#include <kvf/color.hpp>

namespace le {
/// \brief Instance data for instanced rendering.
struct RenderInstance {
	Transform transform{};
	kvf::Color tint{kvf::white_v};
};
} // namespace le
