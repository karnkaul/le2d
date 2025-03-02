#pragma once
#include <djson/json.hpp>
#include <le2d/animation.hpp>
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

void from_json(dj::Json const& json, Transform& transform);
void to_json(dj::Json& json, Transform const& transform);

void from_json(dj::Json const& json, Animation::Keyframe& keyframe);
void to_json(dj::Json& json, Animation::Keyframe const& keyframe);

void from_json(dj::Json const& json, Animation& animation);
void to_json(dj::Json& json, Animation const& animation);

void from_json(dj::Json const& json, Flipbook::Keyframe& keyframe);
void to_json(dj::Json& json, Flipbook::Keyframe const& keyframe);

void from_json(dj::Json const& json, Flipbook& flipbook);
void to_json(dj::Json& json, Flipbook const& flipbook);
} // namespace le
