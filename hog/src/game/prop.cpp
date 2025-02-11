#include <game/prop.hpp>

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
