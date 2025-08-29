#pragma once
#include <le2d/drawable/drawable.hpp>
#include <vector>

namespace le {
/// \brief Interface for drawable primitives.
class IDrawPrimitive : public IDrawable {
  public:
	[[nodiscard]] virtual auto get_instances() const -> std::span<RenderInstance const> = 0;
	[[nodiscard]] virtual auto to_primitive() const -> Primitive = 0;

	void draw(IRenderer& renderer) const final { renderer.draw(to_primitive(), get_instances()); }
};

/// \brief Base class for Draw Primitives using a single Render Instance.
template <std::derived_from<IDrawPrimitive> Type>
class SingleDrawPrimitive : public Type, public RenderInstance {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return {static_cast<RenderInstance const*>(this), 1}; }
};

/// \brief Base class for Draw Primitives using a vector of Render Instances.
template <std::derived_from<IDrawPrimitive> Type>
class InstancedDrawPrimitive : public Type {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return instances; }

	std::vector<RenderInstance> instances{};
};
} // namespace le
