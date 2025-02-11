#pragma once
#include <kvf/rect.hpp>
#include <string>

namespace hog {
struct Collectible {
	std::size_t prop_index{};
	std::string description{};
	kvf::UvRect icon_uv{kvf::uv_rect_v};

	bool collected{};
};
} // namespace hog
