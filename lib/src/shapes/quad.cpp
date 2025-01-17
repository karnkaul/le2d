#include <le2d/shapes/quad.hpp>

namespace le::shape {
namespace {
constexpr std::size_t lb_v{0};
constexpr std::size_t rb_v{1};
constexpr std::size_t rt_v{2};
constexpr std::size_t lt_v{3};
} // namespace

auto Quad::get_rect() const -> kvf::Rect<> { return kvf::Rect<>{.lt = m_vertices[lt_v].position, .rb = m_vertices[rb_v].position}; }

void Quad::set_rect(kvf::Rect<> const& rect, kvf::UvRect const& uv) {
	m_vertices[lb_v] = Vertex{.position = rect.bottom_left(), .uv = uv.bottom_left()};
	m_vertices[rb_v] = Vertex{.position = rect.bottom_right(), .uv = uv.bottom_right()};
	m_vertices[rt_v] = Vertex{.position = rect.top_right(), .uv = uv.top_right()};
	m_vertices[lt_v] = Vertex{.position = rect.top_left(), .uv = uv.top_left()};
}

auto Quad::get_uv() const -> kvf::UvRect { return {.lt = m_vertices[lt_v].uv, .rb = m_vertices[rb_v].uv}; }

auto Quad::get_primitive() const -> Primitive {
	return Primitive{
		.vertices = m_vertices,
		.indices = indices_v,
		.texture = texture.get(),
	};
}
} // namespace le::shape
