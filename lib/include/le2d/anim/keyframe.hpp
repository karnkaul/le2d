#pragma once
#include <kvf/time.hpp>

namespace le::anim {
template <typename PayloadT>
struct Keyframe {
	kvf::Seconds timestamp{};
	PayloadT payload{};
};
} // namespace le::anim
