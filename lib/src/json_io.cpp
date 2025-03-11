#include <le2d/json_io.hpp>

namespace {
using namespace le;

template <typename PayloadT>
void do_from_json(dj::Json const& json, anim::Animation<PayloadT>& animation) {
	from_json(json["name"], animation.name);
	if (auto const& repeat = json["repeat"]) { from_json(repeat, animation.repeat); }
	auto timeline = anim::Timeline<PayloadT>{};
	auto const& in_timeline = json["timeline"];
	from_json(in_timeline["duration"], timeline.duration);
	auto const& in_keyframes = in_timeline["keyframes"].array_view();
	timeline.keyframes.reserve(in_keyframes.size());
	for (auto const& in_keyframe : in_keyframes) { from_json(in_keyframe, timeline.keyframes.emplace_back()); }
	animation.set_timeline(std::move(timeline));
}

template <typename PayloadT>
void do_to_json(dj::Json& json, anim::Animation<PayloadT> const& animation) {
	to_json(json["name"], animation.name);
	auto const& in_timeline = animation.get_timeline();
	auto& out_timeline = json["timeline"];
	to_json(out_timeline["duration"], in_timeline.duration);
	if (!in_timeline.keyframes.empty()) {
		auto& out_keyframes = out_timeline["keyframes"];
		for (auto const& keyframe : in_timeline.keyframes) { to_json(out_keyframes.push_back({}), keyframe); }
	}
}
} // namespace

void le::from_json(dj::Json const& json, Uri& uri) {
	auto str = std::string{};
	from_json(json, str);
	uri = std::move(str);
}

void le::to_json(dj::Json& json, Uri const& uri) { to_json(json, uri.get_string()); }

void le::from_json(dj::Json const& json, kvf::Seconds& seconds) {
	auto f = float{};
	from_json(json, f);
	seconds = kvf::Seconds{f};
}

void le::to_json(dj::Json& json, kvf::Seconds const& seconds) { to_json(json, seconds.count()); }

void le::from_json(dj::Json const& json, TileId& tile_id) {
	auto id = std::underlying_type_t<TileId>{};
	from_json(json, id);
	tile_id = TileId{id};
}

void le::to_json(dj::Json& json, TileId const& tile_id) { to_json(json, std::to_underlying(tile_id)); }

void le::from_json(dj::Json const& json, Tile& tile) {
	from_json(json["id"], tile.id);
	from_json(json["uv"], tile.uv);
}

void le::to_json(dj::Json& json, Tile const& tile) {
	to_json(json["id"], tile.id);
	to_json(json["uv"], tile.uv);
}

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

void le::from_json(dj::Json const& json, anim::Keyframe<Transform>& keyframe) {
	from_json(json["timestamp"], keyframe.timestamp);
	from_json(json["transform"], keyframe.payload);
}

void le::to_json(dj::Json& json, anim::Keyframe<Transform> const& keyframe) {
	to_json(json["timestamp"], keyframe.timestamp);
	to_json(json["transform"], keyframe.payload);
}

void le::from_json(dj::Json const& json, anim::TransformAnimation& animation) { do_from_json(json, animation); }

void le::to_json(dj::Json& json, anim::TransformAnimation const& animation) { do_to_json(json, animation); }

void le::from_json(dj::Json const& json, anim::Keyframe<TileId>& keyframe) {
	from_json(json["timestamp"], keyframe.timestamp);
	from_json(json["tile"], keyframe.payload);
}

void le::to_json(dj::Json& json, anim::Keyframe<TileId> const& keyframe) {
	to_json(json["timestamp"], keyframe.timestamp);
	to_json(json["tile"], keyframe.payload);
}

void le::from_json(dj::Json const& json, anim::FlipbookAnimation& animation) { do_from_json(json, animation); }

void le::to_json(dj::Json& json, anim::FlipbookAnimation const& animation) { do_to_json(json, animation); }
