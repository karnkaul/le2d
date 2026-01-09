#include "le2d/shape/triangle.hpp"
#include <glm/trigonometric.hpp>
#include <kvf/is_positive.hpp>
#include <cmath>

namespace le::shape {
void Triangle::create(float radius, kvf::Color const color) {
	if (!kvf::is_positive(radius)) { radius = 0.0f; }
	auto angle = -30.0f;
	for (auto& vertex : vertices) {
		auto const cos = std::cos(glm::radians(angle));
		auto const sin = std::sin(glm::radians(angle));
		auto const x = radius * cos;
		auto const y = radius * sin;
		vertex.position = {x, y};
		vertex.color = color.to_linear();
		vertex.uv = {(0.5f * cos) + 0.5f, 0.5f - (0.5f * sin)};
		angle += 120.0f;
	}
}
} // namespace le::shape
