#include <djson/json.hpp>
#include <le2d/asset/store.hpp>
#include <level_info.hpp>
#include <prop.hpp>

namespace hog {
void Prop::load(le::asset::Store const& store, LevelAssets const& assets, PropInfo const& info) {
	name = info.name;
	transform = sprite.instance.transform = info.transform;
	sprite.set_texture(store.get<le::Texture>(assets.textures.at(info.texture)));
	if (info.animation) { animator.set_animation(store.get<le::Animation>(assets.animations.at(*info.animation))); }
	if (info.flipbook) { flipper.set_animation(store.get<le::Flipbook>(assets.flipbooks.at(*info.flipbook))); }
}

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
