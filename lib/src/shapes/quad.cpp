#include <le2d/shapes/quad.hpp>

namespace le::shape {
namespace {
constexpr auto indices_v = std::array{0u, 1u, 2u, 2u, 3u, 0u};
} // namespace

auto Quad::get_rect() const -> kvf::Rect<> { return kvf::Rect<>{.lt = m_vertices[3].position, .rb = m_vertices[1].position}; }

void Quad::set_rect(kvf::Rect<> const& rect) {
	m_vertices[0] = Vertex{.position = rect.bottom_left(), .uv = {0.0f, 1.0f}};
	m_vertices[1] = Vertex{.position = rect.bottom_right(), .uv = {1.0f, 1.0f}};
	m_vertices[2] = Vertex{.position = rect.top_right(), .uv = {1.0f, 0.0f}};
	m_vertices[3] = Vertex{.position = rect.top_left(), .uv = {0.0f, 0.0f}};
}

auto Quad::primitive() const -> Primitive {
	return Primitive{
		.vertices = m_vertices,
		.indices = indices_v,
		.texture = texture,
	};
}
} // namespace le::shape
