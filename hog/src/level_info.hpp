#pragma once
#include <kvf/color.hpp>
#include <le2d/uri.hpp>

namespace dj {
class Json;
} // namespace dj

namespace hog {
struct LevelAssets {
	std::vector<le::Uri> textures{};
};

struct LevelBackground {
	std::size_t texture{};
	kvf::Color color{kvf::black_v};
	float min_scale{0.25f};
	float max_scale{2.0f};
	float default_scale{1.0f};
};

struct LevelInfo {
	std::string name{};
	LevelAssets assets{};
	LevelBackground background{};
};

void from_json(dj::Json const& json, LevelAssets& level_assets);
void from_json(dj::Json const& json, LevelBackground& level_background);
void from_json(dj::Json const& json, LevelInfo& level_info);
} // namespace hog
