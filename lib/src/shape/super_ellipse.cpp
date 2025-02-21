#include <glm/trigonometric.hpp>
#include <le2d/shape/super_ellipse.hpp>
#include <cmath>

namespace le::shape {
void SuperEllipse::create(Params const& params) {
	m_verts.clear();
	m_params = params;

	auto const color = params.color.to_linear();
	auto const step = 360.0f / float(params.resolution);
	m_verts.vertices.push_back(Vertex{.color = color, .uv = glm::vec2{0.5f}});
	auto const a = 0.5f * params.size.x;
	auto const b = 0.5f * params.size.y;
	for (auto angle = 0.0f; angle < 360.0f + step; angle += step) {
		auto const theta = glm::radians(angle);
		auto const cos = std::cos(theta);
		auto const sin = std::sin(theta);
		auto const left = std::pow(cos / a, params.exponent);
		auto const right = std::pow(sin / b, params.exponent);
		auto const r = std::pow(std::abs(left) + std::abs(right), -1.0f / params.exponent);
		auto vertex = Vertex{.position = {r * cos, r * sin}, .color = color};
		vertex.uv = {(vertex.position.x / params.size.x) + 0.5f, 0.5f - (vertex.position.y / params.size.y)};
		m_verts.vertices.push_back(vertex);
	}
}
} // namespace le::shape
