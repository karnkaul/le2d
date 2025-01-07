#pragma once
#include <kvf/color.hpp>
#include <le2d/transform.hpp>

namespace le {
struct RenderInstance {
	Transform transform{};
	kvf::Color tint{kvf::white_v};
};
} // namespace le
