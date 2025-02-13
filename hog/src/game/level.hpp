#pragma once
#include <kvf/rect.hpp>
#include <le2d/animation.hpp>
#include <le2d/drawable/shape.hpp>
#include <string>

namespace hog {
struct Prop {
	void tick(kvf::Seconds dt);
	void draw(le::Renderer& renderer) const { sprite.draw(renderer); }

	std::string_view name{};
	le::drawable::Sprite sprite{};
	le::Transform transform{};
	le::Animator animator{};
	le::FlipAnimator flipper{};
};

struct Collectible {
	std::size_t prop_index{};
	std::string description{};
	kvf::UvRect icon_uv{kvf::uv_rect_v};

	bool collected{};
};

struct Level {
	std::vector<Prop> props{};
	std::vector<Collectible> collectibles{};
};
} // namespace hog

namespace le::asset {
class Store;
} // namespace le::asset

namespace hog {
struct LevelInfo;

[[nodiscard]] auto build_level(le::asset::Store const& store, LevelInfo const& info) -> Level;
} // namespace hog
