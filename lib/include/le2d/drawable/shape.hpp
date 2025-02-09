#pragma once
#include <le2d/drawable/drawable.hpp>
#include <le2d/shape/line_rect.hpp>
#include <le2d/shape/quad.hpp>
#include <le2d/shape/sprite.hpp>
#include <le2d/shape/super_ellipse.hpp>
#include <le2d/shape/text.hpp>
#include <le2d/vertex_bounds.hpp>

namespace le::drawable {
template <ShapeT Type>
class IShape : public Type, public IDrawable {
  public:
	using Type::Type;

	[[nodiscard]] virtual auto get_instances() const -> std::span<RenderInstance const> = 0;

	void draw(Renderer& renderer) const override { renderer.draw(this->get_primitive(), get_instances()); }
};

template <ShapeT Type>
class Shape : public IShape<Type> {
  public:
	using IShape<Type>::IShape;

	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return {&instance, 1}; }

	[[nodiscard]] auto bounding_rect() const -> kvf::Rect<> { return vertex_bounds(this->get_primitive().vertices, instance.transform.to_model()); }

	RenderInstance instance{};
};

template <ShapeT Type>
class InstancedShape : public IShape<Type> {
  public:
	using IShape<Type>::IShape;

	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return instances; }

	std::vector<RenderInstance> instances{};
};

using Quad = Shape<shape::Quad>;
using InstancedQuad = InstancedShape<shape::Quad>;

using Sprite = Shape<shape::Sprite>;
using InstancedSprite = InstancedShape<shape::Sprite>;

using Text = Shape<shape::Text>;
using InstancedText = InstancedShape<shape::Text>;

using LineRect = Shape<shape::LineRect>;
using InstancedLineRect = InstancedShape<shape::LineRect>;

using SuperEllipse = Shape<shape::SuperEllipse>;
using InstancedSuperEllipse = InstancedShape<shape::SuperEllipse>;
} // namespace le::drawable
