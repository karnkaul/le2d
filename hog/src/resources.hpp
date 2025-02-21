#pragma once
#include <le2d/asset/store.hpp>
#include <le2d/font.hpp>
#include <le2d/texture.hpp>

namespace hog {
class Resources : public le::asset::Store {
  public:
	explicit Resources(le::Font main_font) : main_font(std::move(main_font)) {}

	le::Font main_font;
};
} // namespace hog
