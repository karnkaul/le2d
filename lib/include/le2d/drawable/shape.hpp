#pragma once
#include "le2d/drawable/draw_instance.hpp"
#include "le2d/shape/circle.hpp"
#include "le2d/shape/quad.hpp"
#include "le2d/shape/sector.hpp"
#include "le2d/shape/super_ellipse.hpp"
#include "le2d/shape/triangle.hpp"

namespace le::drawable {
using TriangleGeometry = DrawGeometry<shape::Triangle>;
/// \brief Triangle drawable.
class Triangle : public DrawInstance<TriangleGeometry> {};
/// \brief Triangle drawable for multiple instances.
class InstancedTriangle : public DrawInstances<TriangleGeometry> {};

using QuadGeometry = DrawGeometry<shape::Quad>;
/// \brief Quad drawable.
class Quad : public DrawInstance<QuadGeometry> {};
/// \brief Quad drawable for multiple instances.
class InstancedQuad : public DrawInstances<QuadGeometry> {};

using LineRectGeometry = DrawGeometry<shape::LineRect>;
/// \brief LineRect drawable.
class LineRect : public DrawInstance<LineRectGeometry> {};
/// \brief LineRect drawable for multiple instances.
class InstancedLineRect : public DrawInstances<LineRectGeometry> {};

using SectorGeometry = DrawGeometry<shape::Sector>;
/// \brief Sector drawable.
class Sector : public DrawInstance<SectorGeometry> {};
/// \brief Sector drawable for multiple instances.
class InstancedSector : public DrawInstances<SectorGeometry> {};

using CircleGeometry = DrawGeometry<shape::Circle>;
/// \brief Circle drawable.
class Circle : public DrawInstance<CircleGeometry> {};
/// \brief Circle drawable for multiple instances.
class InstancedCircle : public DrawInstances<CircleGeometry> {};

using SuperEllipseGeometry = DrawGeometry<shape::SuperEllipse>;
/// \brief SuperEllipse drawable.
class SuperEllipse : public DrawInstance<SuperEllipseGeometry> {};
/// \brief SuperEllipse drawable for multiple instances.
class InstancedSuperEllipse : public DrawInstances<SuperEllipseGeometry> {};
} // namespace le::drawable
