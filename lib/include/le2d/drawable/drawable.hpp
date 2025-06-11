#pragma once
#include <le2d/renderer.hpp>

namespace le {
/// \brief Interface for drawable types.
class IDrawable : public klib::Polymorphic {
  public:
	virtual void draw(Renderer& renderer) const = 0;
};
} // namespace le
