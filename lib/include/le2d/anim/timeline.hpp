#pragma once
#include <le2d/anim/keyframe.hpp>
#include <vector>

namespace le::anim {
template <typename PayloadT>
struct Timeline {
	std::vector<Keyframe<PayloadT>> keyframes{};
	kvf::Seconds duration{};
};
} // namespace le::anim
