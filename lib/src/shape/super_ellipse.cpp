#include <glm/trigonometric.hpp>
#include <le2d/shape/super_ellipse.hpp>
#include <cmath>

namespace le::shape {
void SuperEllipse::create(Params const& params) {
	m_verts.clear();
	m_params = params;

	auto const step = 360.0f / float(params.resolution);
	m_verts.vertices.push_back(Vertex{.color = params.color});
	auto const a = 0.5f * params.size.x;
	auto const b = 0.5f * params.size.y;
	for (auto angle = 0.0f; angle < 360.0f + step; angle += step) {
		auto const theta = glm::radians(angle);
		auto const cos = std::cos(theta);
		auto const sin = std::sin(theta);
		auto const left = std::pow(cos / a, params.exponent);
		auto const right = std::pow(sin / b, params.exponent);
		auto const r = std::pow(std::abs(left) + std::abs(right), -1.0f / params.exponent);
		auto const vertex = Vertex{.position = {r * cos, r * sin}, .color = params.color};
		m_verts.vertices.push_back(vertex);
	}
}

auto SuperEllipse::get_primitive() const -> Primitive {
	return Primitive{
		.vertices = m_verts.vertices,
		.topology = vk::PrimitiveTopology::eTriangleFan,
		.texture = texture,
	};
}
} // namespace le::shape
