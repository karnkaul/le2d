#include <game/level_info.hpp>
#include <klib/concepts.hpp>
#include <kvf/util.hpp>
#include <le2d/json_io.hpp>

namespace {
template <klib::NumberT Type>
void optional_from_json(dj::Json const& json, std::optional<Type>& t) {
	if (!json.is_number()) { return; }
	t.emplace();
	from_json(json, *t);
}
} // namespace

void hog::from_json(dj::Json const& json, LevelAssets& level_assets) {
	for (auto const& in_texture : json["textures"].array_view()) { level_assets.textures.emplace_back(in_texture.as<std::string>()); }
	for (auto const& in_animation : json["animations"].array_view()) { level_assets.animations.emplace_back(in_animation.as<std::string>()); }
	for (auto const& in_flipbook : json["flipbooks"].array_view()) { level_assets.flipbooks.emplace_back(in_flipbook.as<std::string>()); }
}

void hog::from_json(dj::Json const& json, LevelBackground& level_background) {
	from_json(json["texture"], level_background.texture, level_background.texture);
	if (auto const& color = json["color"]) { level_background.color = kvf::util::color_from_hex(color.as_string()); }
	from_json(json["min_scale"], level_background.min_scale, level_background.min_scale);
	from_json(json["max_scale"], level_background.max_scale, level_background.max_scale);
	from_json(json["default_scale"], level_background.default_scale, level_background.default_scale);
}

void hog::from_json(dj::Json const& json, PropInfo& prop_info) {
	from_json(json["name"], prop_info.name);
	le::from_json(json["transform"], prop_info.transform);
	if (auto const& sprite_size = json["sprite_size"]) { le::from_json(sprite_size, prop_info.sprite_size); }
	from_json(json["texture"], prop_info.texture);
	optional_from_json(json["animation"], prop_info.animation);
	optional_from_json(json["flipbook"], prop_info.flipbook);
}

void hog::from_json(dj::Json const& json, CollectibleInfo& collectible_info) {
	from_json(json["prop"], collectible_info.prop);
	from_json(json["description"], collectible_info.description);
	if (auto const& icon_uv = json["icon_uv"]) { le::from_json(icon_uv, collectible_info.icon_uv); }
}

void hog::from_json(dj::Json const& json, LevelInfo& level_info) {
	from_json(json["name"], level_info.name);
	from_json(json["assets"], level_info.assets);
	from_json(json["background"], level_info.background);
	auto const& props = json["props"].array_view();
	level_info.props.reserve(props.size());
	for (auto const& prop : props) { from_json(prop, level_info.props.emplace_back()); }
	auto const& collectibles = json["collectibles"].array_view();
	level_info.collectibles.reserve(collectibles.size());
	for (auto const& collectible : collectibles) { from_json(collectible, level_info.collectibles.emplace_back()); }
}
