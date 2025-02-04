#include <djson/json.hpp>
#include <level_info.hpp>

void hog::from_json(dj::Json const& json, LevelAssets& level_assets) {
	for (auto const& in_texture : json["textures"].array_view()) { level_assets.textures.emplace_back(in_texture.as<std::string>()); }
}

void hog::from_json(dj::Json const& json, LevelBackground& level_background) {
	from_json(json["texture"], level_background.texture, level_background.texture);
	from_json(json["min_scale"], level_background.min_scale, level_background.min_scale);
	from_json(json["max_scale"], level_background.max_scale, level_background.max_scale);
	from_json(json["default_scale"], level_background.default_scale, level_background.default_scale);
}

void hog::from_json(dj::Json const& json, LevelInfo& level_info) {
	from_json(json["name"], level_info.name);
	from_json(json["assets"], level_info.assets);
	from_json(json["background"], level_info.background);
}
