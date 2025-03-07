#pragma once
#include <le2d/drawable/geometry.hpp>
#include <le2d/shape/quad.hpp>
#include <le2d/shape/super_ellipse.hpp>
#include <le2d/shape/triangle.hpp>

namespace le::drawable {
using Triangle = Geometry<shape::Triangle>;
using InstancedTriangle = InstancedGeometry<shape::Triangle>;

using Quad = Geometry<shape::Quad>;
using InstancedQuad = InstancedGeometry<shape::Quad>;

using LineRect = Geometry<shape::LineRect>;
using InstancedLineRect = InstancedGeometry<shape::LineRect>;

using SuperEllipse = Geometry<shape::SuperEllipse>;
using InstancedSuperEllipse = InstancedGeometry<shape::SuperEllipse>;
} // namespace le::drawable
