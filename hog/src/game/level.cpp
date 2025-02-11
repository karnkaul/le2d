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
		for (auto const& prop : info.props) { level.props.push_back(create_prop(prop)); }
		level.collectibles.reserve(info.collectibles.size());
		for (auto const& collectible : info.collectibles) { level.collectibles.push_back(create_collectible(collectible)); }
	}

	[[nodiscard]] auto create_prop(PropInfo const& prop_info) const -> Prop {
		auto ret = Prop{.name = prop_info.name};
		ret.transform = ret.sprite.instance.transform = prop_info.transform;
		ret.sprite.set_texture(store.get<le::Texture>(info.assets.textures.at(prop_info.texture)));
		if (prop_info.animation) { ret.animator.set_animation(store.get<le::Animation>(info.assets.animations.at(*prop_info.animation))); }
		if (prop_info.flipbook) { ret.flipper.set_animation(store.get<le::Flipbook>(info.assets.flipbooks.at(*prop_info.flipbook))); }
		return ret;
	}

	[[nodiscard]] static auto create_collectible(CollectibleInfo const& collectible_info) -> Collectible {
		return Collectible{
			.prop_index = collectible_info.prop,
			.description = collectible_info.description,
			.icon_uv = collectible_info.icon_uv,
		};
	}
};
} // namespace
} // namespace hog

auto hog::build_level(le::asset::Store const& store, LevelInfo const& info) -> Level {
	auto ret = Level{};
	Builder{.store = store, .info = info, .level = ret}.build();
	return ret;
}
