#include <glm/glm.hpp>
#include <klib/assert.hpp>
#include <le2d/animation.hpp>

namespace le {
namespace {
template <typename KeyframeT>
auto get_upper_bound(std::span<KeyframeT const> timeline, kvf::Seconds const time) {
	return std::ranges::upper_bound(timeline, time, std::ranges::less{}, [](KeyframeT const& f) { return f.timestamp; });
}

template <typename TimelineT>
auto get_bounds(TimelineT const& timeline, kvf::Seconds const time) {
	KLIB_ASSERT(!timeline.empty());
	using type = typename TimelineT::value_type;
	using Ret = std::pair<type const&, type const&>;
	auto const it = get_upper_bound(timeline, time);
	if (it == timeline.begin()) { return Ret{*it, *it}; }
	if (it == timeline.end()) { return Ret{*(it - 1), *(it - 1)}; }
	return Ret{*(it - 1), *it};
}

template <typename KeyframeT>
auto get_keyframe_at(std::span<KeyframeT const> timeline, kvf::Seconds const time) -> KeyframeT const* {
	if (timeline.empty()) { return nullptr; }
	auto const it = get_upper_bound(timeline, time);
	if (it == timeline.begin()) { return &*it; }
	return &*(it - 1);
}
} // namespace

auto Animation::sample(kvf::Seconds const time) const -> Transform {
	auto const timeline = get_timeline();
	if (timeline.empty()) { return {}; }
	auto const [left, right] = get_bounds(timeline, time);
	if (left.timestamp == right.timestamp) { return left.payload; }
	auto const total_t = right.timestamp - left.timestamp;
	auto const t = (time - left.timestamp) / total_t;
	return Transform{
		.position = glm::mix(left.payload.position, right.payload.position, t),
		.orientation = std::lerp(left.payload.orientation, right.payload.orientation, t),
		.scale = glm::mix(left.payload.scale, right.payload.scale, t),
	};
}

auto Flipbook::sample(kvf::Seconds const time) const -> kvf::UvRect {
	auto const* keyframe = get_keyframe_at(get_timeline(), time);
	if (keyframe == nullptr) { return {}; }
	return keyframe->payload;
}
} // namespace le
