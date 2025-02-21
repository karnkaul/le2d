#pragma once
#include <le2d/drawable/drawable.hpp>
#include <vector>

namespace le {
class IDrawPrimitive : public IDrawable {
  public:
	[[nodiscard]] virtual auto get_instances() const -> std::span<RenderInstance const> = 0;
	[[nodiscard]] virtual auto to_primitive() const -> Primitive = 0;

	void draw(Renderer& renderer) const final { renderer.draw(to_primitive(), get_instances()); }
};

template <std::derived_from<IDrawPrimitive> Type>
class SingleDrawPrimitive : public Type, public RenderInstance {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return {static_cast<RenderInstance const*>(this), 1}; }
};

template <std::derived_from<IDrawPrimitive> Type>
class InstancedDrawPrimitive : public Type {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return instances; }

	std::vector<RenderInstance> instances{};
};
} // namespace le
