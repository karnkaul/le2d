#pragma once
#include <kvf/time.hpp>

namespace le::anim {
/// \brief Class template for an animation keyframe.
template <typename PayloadT>
struct Keyframe {
	kvf::Seconds timestamp{};
	PayloadT payload{};
};
} // namespace le::anim
