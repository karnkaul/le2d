#pragma once
#include <le2d/renderer.hpp>
#include <le2d/shapes/quad.hpp>

namespace le {
class IDrawable : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_primitive() const -> Primitive = 0;
	[[nodiscard]] virtual auto get_instances() const -> std::span<RenderInstance const> = 0;

	void draw(Renderer& renderer, UserDrawData const& user_data = {}) const { renderer.draw(get_primitive(), get_instances(), user_data); }
};

template <ShapeT Type>
class IDrawableShape : public Type, public IDrawable {
  public:
	[[nodiscard]] auto get_primitive() const -> Primitive final { return this->Type::primitive(); }
};

template <ShapeT Type>
class DrawableShape : public IDrawableShape<Type> {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return {&instance, 1}; }

	RenderInstance instance{};
};

template <ShapeT Type>
class InstancedShape : public IDrawableShape<Type> {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return instances; }

	std::vector<RenderInstance> instances{};
};

using Quad = DrawableShape<shape::Quad>;
using InstancedQuad = InstancedShape<shape::Quad>;
} // namespace le
