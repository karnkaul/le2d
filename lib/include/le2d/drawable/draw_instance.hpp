#pragma once
#include "le2d/drawable/draw_primitive.hpp"
#include "le2d/drawable/drawable.hpp"
#include "le2d/vertex_bounds.hpp"
#include <vector>

namespace le {
/// \brief Concrete drawable that inherits from DrawPrimitiveT, provides a single RenderInstance.
template <std::derived_from<IDrawPrimitive> DrawPrimitiveT>
class DrawInstance : public DrawPrimitiveT, public le::IDrawable {
  public:
	void draw(le::IRenderer& renderer) const final { renderer.draw(this->get_geometry().to_primitive(this->get_texture()), {&instance, 1}); }

	[[nodiscard]] auto bounding_rect() const -> kvf::Rect<> { return vertex_bounds(this->get_geometry().get_vertices(), instance.transform.to_model()); }

	le::RenderInstance instance{};
};

/// \brief Concrete drawable that inherits from DrawPrimitiveT, provides a list of RenderInstances.
template <std::derived_from<IDrawPrimitive> DrawPrimitiveT>
class DrawInstances : public DrawPrimitiveT, public le::IDrawable {
  public:
	void draw(le::IRenderer& renderer) const final { renderer.draw(this->get_geometry().to_primitive(this->get_texture()), instances); }

	std::vector<le::RenderInstance> instances{};
};
} // namespace le
