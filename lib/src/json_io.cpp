#include <le2d/json_io.hpp>

namespace {
template <typename AnimationT>
void do_from_json(dj::Json const& json, AnimationT& animation) {
	from_json(json["name"], animation.name);
	auto timeline = std::vector<typename AnimationT::Keyframe>{};
	auto const& in_timeline = json["timeline"].array_view();
	timeline.reserve(in_timeline.size());
	for (auto const& in_keyframe : in_timeline) { from_json(in_keyframe, timeline.emplace_back()); }
	animation.set_timeline(std::move(timeline));
}

template <typename AnimationT>
void do_to_json(dj::Json& json, AnimationT const& animation) {
	to_json(json["name"], animation.name);
	for (auto const& keyframe : animation.get_timeline()) { to_json(json["timeline"].push_back({}), keyframe); }
}
} // namespace

void le::from_json(dj::Json const& json, kvf::Seconds& seconds) {
	auto f = float{};
	from_json(json, f);
	seconds = kvf::Seconds{f};
}

void le::to_json(dj::Json& json, kvf::Seconds const& seconds) { to_json(json, seconds.count()); }

void le::from_json(dj::Json const& json, Transform& transform) {
	from_json(json["position"], transform.position);
	from_json(json["orientation"], transform.orientation);
	from_json(json["scale"], transform.scale);
}

void le::to_json(dj::Json& json, Transform const& transform) {
	to_json(json["position"], transform.position);
	to_json(json["orientation"], transform.orientation);
	to_json(json["scale"], transform.scale);
}

void le::from_json(dj::Json const& json, Animation::Keyframe& keyframe) {
	from_json(json["timestamp"], keyframe.timestamp);
	from_json(json["transform"], keyframe.payload);
}

void le::to_json(dj::Json& json, Animation::Keyframe const& keyframe) {
	to_json(json["timestamp"], keyframe.timestamp);
	to_json(json["transform"], keyframe.payload);
}

void le::from_json(dj::Json const& json, Animation& animation) { do_from_json(json, animation); }

void le::to_json(dj::Json& json, Animation const& animation) { do_to_json(json, animation); }

void le::from_json(dj::Json const& json, Flipbook::Keyframe& keyframe) {
	from_json(json["timestamp"], keyframe.timestamp);
	from_json(json["rect"], keyframe.payload);
}

void le::to_json(dj::Json& json, Flipbook::Keyframe const& keyframe) {
	to_json(json["timestamp"], keyframe.timestamp);
	to_json(json["rect"], keyframe.payload);
}

void le::from_json(dj::Json const& json, Flipbook& flipbook) { do_from_json(json, flipbook); }

void le::to_json(dj::Json& json, Flipbook const& flipbook) { do_to_json(json, flipbook); }
