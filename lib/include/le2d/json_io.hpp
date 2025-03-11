#pragma once
#include <djson/json.hpp>
#include <le2d/anim/animation.hpp>
#include <le2d/tile.hpp>
#include <le2d/uri.hpp>

namespace le {
template <typename Type, glm::length_t Length>
void from_json(dj::Json const& json, glm::vec<Length, Type>& vec) {
	if (!json.is_array()) { return; }
	from_json(json[0], vec.x);
	if constexpr (Length > 1) { from_json(json[1], vec.y); }
	if constexpr (Length > 2) { from_json(json[2], vec.z); }
}

template <typename Type, glm::length_t Length>
void to_json(dj::Json& json, glm::vec<Length, Type> const& vec) {
	to_json(json[0], vec.x);
	if constexpr (Length > 1) { to_json(json[1], vec.y); }
	if constexpr (Length > 2) { to_json(json[2], vec.z); }
}

template <typename Type>
void from_json(dj::Json const& json, kvf::Rect<Type>& rect) {
	from_json(json["lt"], rect.lt);
	from_json(json["rb"], rect.rb);
}

template <typename Type>
void to_json(dj::Json& json, kvf::Rect<Type> const& rect) {
	to_json(json["lt"], rect.lt);
	to_json(json["rb"], rect.rb);
}

void from_json(dj::Json const& json, Uri& uri);
void to_json(dj::Json& json, Uri const& uri);

void from_json(dj::Json const& json, kvf::Seconds& seconds);
void to_json(dj::Json& json, kvf::Seconds const& seconds);

void from_json(dj::Json const& json, TileId& tile_id);
void to_json(dj::Json& json, TileId const& tile_id);

void from_json(dj::Json const& json, Tile& tile);
void to_json(dj::Json& json, Tile const& tile);

void from_json(dj::Json const& json, Transform& transform);
void to_json(dj::Json& json, Transform const& transform);

void from_json(dj::Json const& json, anim::Keyframe<Transform>& keyframe);
void to_json(dj::Json& json, anim::Keyframe<Transform> const& keyframe);

void from_json(dj::Json const& json, anim::Timeline<Transform>& timeline);
void to_json(dj::Json& json, anim::Timeline<Transform> const& timeline);

void from_json(dj::Json const& json, anim::Animation<Transform>& animation);
void to_json(dj::Json& json, anim::Animation<Transform> const& animation);

void from_json(dj::Json const& json, anim::Keyframe<TileId>& keyframe);
void to_json(dj::Json& json, anim::Keyframe<TileId> const& keyframe);

void from_json(dj::Json const& json, anim::Timeline<TileId>& timeline);
void to_json(dj::Json& json, anim::Timeline<TileId> const& timeline);

void from_json(dj::Json const& json, anim::Animation<TileId>& animation);
void to_json(dj::Json& json, anim::Animation<TileId> const& animation);
} // namespace le
