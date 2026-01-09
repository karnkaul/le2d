#pragma once
#include "le2d/drawable/draw_primitive.hpp"
#include "le2d/geometry.hpp"
#include "le2d/vertex_bounds.hpp"

namespace le::drawable {
/// \brief Interface for drawable figures.
class IFigure : public IDrawPrimitive {};

/// \brief Base class for drawable Figure.
template <std::derived_from<IGeometry> Type>
class FigureBase : public Type, public IFigure {
  public:
	using Type::Type;

	[[nodiscard]] auto to_primitive() const -> Primitive final {
		return Primitive{
			.vertices = this->get_vertices(),
			.indices = this->get_indices(),
			.topology = this->get_topology(),
			.texture = texture,
		};
	}

	ITextureBase const* texture{};
};

/// \brief Base class for Figures using a single Render Instance.
template <std::derived_from<IGeometry> Type>
class Figure : public SingleDrawPrimitive<FigureBase<Type>> {
  public:
	using SingleDrawPrimitive<FigureBase<Type>>::SingleDrawPrimitive;

	/// \returns Bounding rect of this figure in world space.
	[[nodiscard]] auto bounding_rect() const -> kvf::Rect<> { return vertex_bounds(this->to_primitive().vertices, this->transform.to_model()); }
};

/// \brief Base class for Figures using a vector of Render Instances.
template <std::derived_from<IGeometry> Type>
class InstancedFigure : public InstancedDrawPrimitive<FigureBase<Type>> {
  public:
	using InstancedDrawPrimitive<FigureBase<Type>>::InstancedDrawPrimitive;
};
} // namespace le::drawable
