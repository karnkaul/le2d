#pragma once
#include "le2d/geometry.hpp"

namespace le {
/// \brief Interface for draw primitives (geometry and texture).
class IDrawPrimitive : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_geometry() const -> IGeometry const& = 0;
	[[nodiscard]] virtual auto get_texture() const -> klib::Ptr<ITextureBase const> = 0;
};

/// \brief Concrete draw primitive storing GeometryT and pointer to texture.
template <std::derived_from<IGeometry> GeometryT>
class DrawGeometry : public IDrawPrimitive {
  public:
	[[nodiscard]] auto get_geometry() const -> IGeometry const& final { return geometry; }
	[[nodiscard]] auto get_texture() const -> klib::Ptr<ITextureBase const> final { return texture; }

	GeometryT geometry{};
	klib::Ptr<ITextureBase const> texture{};
};
} // namespace le
