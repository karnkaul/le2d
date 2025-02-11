#include <game/level_info.hpp>
#include <game/prop.hpp>
#include <le2d/asset/store.hpp>

namespace hog {
void Prop::tick(kvf::Seconds dt) {
	if (animator.has_animation()) {
		animator.tick(dt);
		sprite.instance.transform = le::Transform::accumulate(transform, animator.get_payload());
	}
	if (flipper.has_animation()) {
		flipper.tick(dt);
		sprite.set_uv(flipper.get_payload());
	}
}
} // namespace hog

auto hog::create_prop(le::asset::Store const& asset_store, LevelAssets const& level_assets, PropInfo const& prop_info) -> Prop {
	auto ret = Prop{.name = prop_info.name};
	ret.transform = ret.sprite.instance.transform = prop_info.transform;
	ret.sprite.set_texture(asset_store.get<le::Texture>(level_assets.textures.at(prop_info.texture)));
	if (prop_info.animation) { ret.animator.set_animation(asset_store.get<le::Animation>(level_assets.animations.at(*prop_info.animation))); }
	if (prop_info.flipbook) { ret.flipper.set_animation(asset_store.get<le::Flipbook>(level_assets.flipbooks.at(*prop_info.flipbook))); }
	return ret;
}
