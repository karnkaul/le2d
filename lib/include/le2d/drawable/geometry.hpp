#pragma once
#include <le2d/drawable/drawable.hpp>
#include <le2d/geometry.hpp>
#include <le2d/shape/quad.hpp>
#include <le2d/vertex_bounds.hpp>

namespace le::drawable {
template <std::derived_from<IGeometry> Type>
class GeometryBase : public Type, public IDrawable {
  public:
	using Type::Type;

	[[nodiscard]] virtual auto get_instances() const -> std::span<RenderInstance const> = 0;

	[[nodiscard]] auto to_primitive() const -> Primitive {
		return Primitive{
			.vertices = this->get_vertices(),
			.indices = this->get_indices(),
			.topology = this->get_topology(),
			.texture = texture,
		};
	}

	void draw(Renderer& renderer) const override { renderer.draw(to_primitive(), get_instances()); }

	ITexture const* texture{};
};

template <std::derived_from<IGeometry> Type>
class Geometry : public GeometryBase<Type>, public RenderInstance {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return {static_cast<RenderInstance const*>(this), 1}; }

	[[nodiscard]] auto bounding_rect() const -> kvf::Rect<> { return vertex_bounds(this->to_primitive().vertices, transform.to_model()); }
};

template <std::derived_from<IGeometry> Type>
class InstancedGeometry : public GeometryBase<Type> {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return instances; }

	std::vector<RenderInstance> instances{};
};
} // namespace le::drawable
