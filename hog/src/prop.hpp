#pragma once
#include <le2d/animation.hpp>
#include <le2d/drawable/shape.hpp>

namespace hog {
class Prop {
  public:
	void tick(kvf::Seconds dt);
	void draw(le::Renderer& renderer) const { sprite.draw(renderer); }

	std::string_view name{};
	le::drawable::Sprite sprite{};
	le::Transform transform{};
	le::Animator animator{};
	le::FlipAnimator flipper{};
};
} // namespace hog

namespace le::asset {
class Store;
}

namespace hog {
struct LevelAssets;
struct PropInfo;

[[nodiscard]] auto create_prop(le::asset::Store const& asset_store, LevelAssets const& level_assets, PropInfo const& prop_info) -> Prop;
} // namespace hog
