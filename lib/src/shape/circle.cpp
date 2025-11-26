#include "le2d/shape/circle.hpp"
#include <glm/trigonometric.hpp>
#include <cmath>

namespace le::shape {
void Circle::create(float const diameter, kvf::Color const color, std::int32_t const resolution) {
	m_verts.clear();
	m_diameter = diameter;

	auto const fcolor = color.to_linear();
	auto const step = 360.0f / float(resolution);
	m_verts.vertices.push_back(Vertex{.color = fcolor, .uv = glm::vec2{0.5f}});
	auto const r = diameter * 0.5f;
	for (auto angle = 0.0f; angle < 360.0f + step; angle += step) {
		auto const theta = glm::radians(angle);
		auto const cos = std::cos(theta);
		auto const sin = std::sin(theta);
		auto vertex = Vertex{.position = {r * cos, r * sin}, .color = fcolor};
		vertex.uv = {cos + 0.5f, 0.5f - sin};
		m_verts.vertices.push_back(vertex);
	}
}
} // namespace le::shape
