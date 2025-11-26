#pragma once
#include "le2d/drawable/figure.hpp"
#include "le2d/shape/circle.hpp"
#include "le2d/shape/quad.hpp"
#include "le2d/shape/super_ellipse.hpp"
#include "le2d/shape/triangle.hpp"

namespace le::drawable {
using Triangle = Figure<shape::Triangle>;
using InstancedTriangle = InstancedFigure<shape::Triangle>;

using Quad = Figure<shape::Quad>;
using InstancedQuad = InstancedFigure<shape::Quad>;

using LineRect = Figure<shape::LineRect>;
using InstancedLineRect = InstancedFigure<shape::LineRect>;

using Circle = Figure<shape::Circle>;
using InstancedCircle = InstancedFigure<shape::Circle>;

using SuperEllipse = Figure<shape::SuperEllipse>;
using InstancedSuperEllipse = InstancedFigure<shape::SuperEllipse>;
} // namespace le::drawable
