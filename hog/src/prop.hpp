#pragma once
#include <le2d/animation.hpp>
#include <le2d/drawable/shape.hpp>

namespace le::asset {
class Store;
}

namespace hog {
struct LevelAssets;
struct PropInfo;

class Prop {
  public:
	void load(le::asset::Store const& store, LevelAssets const& assets, PropInfo const& info);

	void tick(kvf::Seconds dt);
	void draw(le::Renderer& renderer) const { sprite.draw(renderer); }

	std::string_view name{};
	le::drawable::Sprite sprite{};
	le::Transform transform{};
	le::Animator<le::Animation> animation{};
	le::Animator<le::Flipbook> flipbook{};
};
} // namespace hog
