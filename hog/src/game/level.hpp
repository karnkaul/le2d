#pragma once
#include <game/collectible.hpp>
#include <game/prop.hpp>

namespace hog {
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
