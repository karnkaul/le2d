#include <game/level.hpp>
#include <game/level_info.hpp>
#include <le2d/asset/store.hpp>

namespace hog {
namespace {
struct Builder {
	le::asset::Store const& store;
	LevelInfo const& info;

	Level& level;

	void build() {
		level.props.reserve(info.props.size());
		for (auto const& prop : info.props) { level.props.push_back(to_prop(prop)); }
		level.collectibles.reserve(info.collectibles.size());
		for (auto const& collectible : info.collectibles) { level.collectibles.push_back(to_collectible(collectible)); }
	}

	[[nodiscard]] auto to_prop(PropInfo const& prop_info) const -> Prop {
		auto ret = Prop{.name = prop_info.name};
		ret.transform = ret.sprite.instance.transform = prop_info.transform;
		ret.sprite.set_texture(store.get<le::Texture>(info.assets.textures.at(prop_info.texture)));
		if (prop_info.animation) { ret.animator.set_animation(store.get<le::Animation>(info.assets.animations.at(*prop_info.animation))); }
		if (prop_info.flipbook) { ret.flipper.set_animation(store.get<le::Flipbook>(info.assets.flipbooks.at(*prop_info.flipbook))); }
		return ret;
	}

	[[nodiscard]] static auto to_collectible(CollectibleInfo const& collectible_info) -> Collectible {
		return Collectible{
			.prop_index = collectible_info.prop,
			.description = collectible_info.description,
			.icon_uv = collectible_info.icon_uv,
		};
	}
};
} // namespace

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

auto hog::build_level(le::asset::Store const& store, LevelInfo const& info) -> Level {
	auto ret = Level{};
	Builder{.store = store, .info = info, .level = ret}.build();
	return ret;
}
