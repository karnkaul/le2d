#pragma once
#include <le2d/drawable/draw_primitive.hpp>
#include <le2d/geometry.hpp>
#include <le2d/shape/quad.hpp>
#include <le2d/vertex_bounds.hpp>

namespace le::drawable {
template <std::derived_from<IGeometry> Type>
class GeometryBase : public Type, public IDrawPrimitive {
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

	ITexture const* texture{};
};

template <std::derived_from<IGeometry> Type>
class Geometry : public SingleDrawPrimitive<GeometryBase<Type>> {
  public:
	[[nodiscard]] auto bounding_rect() const -> kvf::Rect<> { return vertex_bounds(this->to_primitive().vertices, this->transform.to_model()); }
};

template <std::derived_from<IGeometry> Type>
class InstancedGeometry : public InstancedDrawPrimitive<GeometryBase<Type>> {};
} // namespace le::drawable
