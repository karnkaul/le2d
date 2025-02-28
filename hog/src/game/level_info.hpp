#pragma once
#include <kvf/color.hpp>
#include <kvf/rect.hpp>
#include <le2d/transform.hpp>
#include <le2d/uri.hpp>
#include <optional>
#include <vector>

namespace dj {
class Json;
} // namespace dj

namespace hog {
struct LevelAssets {
	std::vector<le::Uri> textures{};
	std::vector<le::Uri> animations{};
	std::vector<le::Uri> flipbooks{};
};

struct LevelBackground {
	std::size_t texture{};
	kvf::Color color{kvf::black_v};
	float min_scale{0.25f};
	float max_scale{2.0f};
	float default_scale{1.0f};
};

struct PropInfo {
	std::string name{};
	le::Transform transform{};
	glm::vec2 sprite_size{200.0f};
	std::size_t texture{};
	std::optional<std::size_t> animation{};
	std::optional<std::size_t> flipbook{};
};

struct CollectibleInfo {
	std::size_t prop{};
	std::string description{};
	kvf::UvRect icon_uv{kvf::uv_rect_v};
};

struct LevelInfo {
	std::string name{};
	LevelAssets assets{};
	LevelBackground background{};
	std::vector<PropInfo> props{};
	std::vector<CollectibleInfo> collectibles{};
};

void from_json(dj::Json const& json, LevelAssets& level_assets);
void from_json(dj::Json const& json, LevelBackground& level_background);
void from_json(dj::Json const& json, PropInfo& prop_info);
void from_json(dj::Json const& json, CollectibleInfo& collectible_info);
void from_json(dj::Json const& json, LevelInfo& level_info);
} // namespace hog
