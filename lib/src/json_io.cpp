#include <le2d/json_io.hpp>

namespace {
template <typename PayloadT>
void timeline_from_json(dj::Json const& json, le::anim::Timeline<PayloadT>& timeline) {
	if (!le::is_json_type(json, timeline)) { return; }
	le::from_json(json["duration"], timeline.duration);
	auto const& in_keyframes = json["keyframes"].array_view();
	timeline.keyframes.reserve(in_keyframes.size());
	for (auto const& in_keyframe : in_keyframes) { from_json(in_keyframe, timeline.keyframes.emplace_back()); }
}

template <typename PayloadT>
void timeline_to_json(dj::Json& json, le::anim::Timeline<PayloadT> const& timeline) {
	le::set_json_type(json, timeline);
	le::to_json(json["duration"], timeline.duration);
	if (!timeline.keyframes.empty()) {
		auto& out_keyframes = json["keyframes"];
		for (auto const& keyframe : timeline.keyframes) { to_json(out_keyframes.push_back({}), keyframe); }
	}
}

template <typename PayloadT>
void animation_from_json(dj::Json const& json, le::anim::Animation<PayloadT>& animation) {
	if (!is_json_type(json, animation)) { return; }
	from_json(json["name"], animation.name);
	if (auto const& repeat = json["repeat"]) { from_json(repeat, animation.repeat); }
	auto timeline = le::anim::Timeline<PayloadT>{};
	from_json(json["timeline"], timeline);
	animation.set_timeline(std::move(timeline));
}

template <typename PayloadT>
void animation_to_json(dj::Json& json, le::anim::Animation<PayloadT> const& animation) {
	set_json_type(json, animation);
	to_json(json["name"], animation.name);
	to_json(json["repeat"], animation.repeat);
	to_json(json["timeline"], animation.get_timeline());
}
} // namespace

auto le::get_json_type_name(dj::Json const& json) -> std::string_view { return json["type_name"].as_string(); }

void le::set_json_type_name(dj::Json& json, std::string_view const type_name) {
	if (type_name.empty()) { return; }
	to_json(json["type_name"], type_name);
}

auto le::json_type_name_match(dj::Json const& json, std::string_view const type_name) -> bool {
	return !type_name.empty() && get_json_type_name(json) == type_name;
}

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

void le::from_json(dj::Json const& json, TileSet& tile_set) {
	if (!is_json_type(json, tile_set)) { return; }
	auto tiles = std::vector<Tile>{};
	auto const& in_tiles = json["tiles"].array_view();
	tiles.reserve(in_tiles.size());
	for (auto const& in_tile : in_tiles) { from_json(in_tile, tiles.emplace_back()); }
	tile_set.set_tiles(std::move(tiles));
}

void le::to_json(dj::Json& json, TileSet const& tile_set) {
	set_json_type(json, tile_set);
	auto const tiles = tile_set.get_tiles();
	for (auto const& tile : tiles) { to_json(json.push_back({}), tile); }
}

void le::from_json(dj::Json const& json, anim::Keyframe<Transform>& keyframe) {
	from_json(json["timestamp"], keyframe.timestamp);
	from_json(json["transform"], keyframe.payload);
}

void le::to_json(dj::Json& json, anim::Keyframe<Transform> const& keyframe) {
	to_json(json["timestamp"], keyframe.timestamp);
	to_json(json["transform"], keyframe.payload);
}

void le::from_json(dj::Json const& json, anim::Timeline<Transform>& timeline) { timeline_from_json(json, timeline); }

void le::to_json(dj::Json& json, anim::Timeline<Transform> const& timeline) { timeline_to_json(json, timeline); }

void le::from_json(dj::Json const& json, anim::Animation<Transform>& animation) { animation_from_json(json, animation); }

void le::to_json(dj::Json& json, anim::Animation<Transform> const& animation) { animation_to_json(json, animation); }

void le::from_json(dj::Json const& json, anim::Keyframe<TileId>& keyframe) {
	from_json(json["timestamp"], keyframe.timestamp);
	from_json(json["tile"], keyframe.payload);
}

void le::to_json(dj::Json& json, anim::Keyframe<TileId> const& keyframe) {
	to_json(json["timestamp"], keyframe.timestamp);
	to_json(json["tile"], keyframe.payload);
}

void le::from_json(dj::Json const& json, anim::Timeline<TileId>& timeline) { timeline_from_json(json, timeline); }

void le::to_json(dj::Json& json, anim::Timeline<TileId> const& timeline) { timeline_to_json(json, timeline); }

void le::from_json(dj::Json const& json, anim::Animation<TileId>& animation) { animation_from_json(json, animation); }

void le::to_json(dj::Json& json, anim::Animation<TileId> const& animation) { animation_to_json(json, animation); }
