#include <djson/json.hpp>
#include <le2d/asset/store.hpp>
#include <level_info.hpp>
#include <prop.hpp>

namespace hog {
void Prop::load(le::asset::Store const& store, LevelAssets const& assets, PropInfo const& info) {
	name = info.name;
	transform = sprite.instance.transform = info.transform;
	sprite.set_texture(store.get<le::Texture>(assets.textures.at(info.texture)));
	if (info.animation) { animation.set_animation(store.get<le::Animation>(assets.animations.at(*info.animation))); }
	if (info.flipbook) { flipbook.set_animation(store.get<le::Flipbook>(assets.flipbooks.at(*info.flipbook))); }
}

void Prop::tick(kvf::Seconds dt) {
	if (animation.has_animation()) {
		animation.tick(dt);
		sprite.instance.transform = le::Transform::accumulate(transform, animation.get_payload());
	}
	if (flipbook.has_animation()) {
		flipbook.tick(dt);
		sprite.set_uv(flipbook.get_payload());
	}
}
} // namespace hog
