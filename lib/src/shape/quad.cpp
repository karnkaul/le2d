#include "le2d/shape/quad.hpp"
#include <kvf/is_positive.hpp>

namespace le::shape {
namespace {
constexpr std::size_t lb_v{0};
constexpr std::size_t rb_v{1};
constexpr std::size_t rt_v{2};
constexpr std::size_t lt_v{3};
} // namespace

auto IQuad::get_rect() const -> kvf::Rect<> { return kvf::Rect<>{.lt = m_vertices[lt_v].position, .rb = m_vertices[rb_v].position}; }

void IQuad::create(glm::vec2 size) {
	if (!kvf::is_positive(size)) { size = {}; }
	create(kvf::Rect<>::from_size(size));
}

void IQuad::create(kvf::Rect<> const& rect, kvf::UvRect const& uv, kvf::Color const color) {
	auto const vec4_color = color.to_linear();
	m_vertices[lb_v] = Vertex{.position = rect.bottom_left(), .color = vec4_color, .uv = uv.bottom_left()};
	m_vertices[rb_v] = Vertex{.position = rect.bottom_right(), .color = vec4_color, .uv = uv.bottom_right()};
	m_vertices[rt_v] = Vertex{.position = rect.top_right(), .color = vec4_color, .uv = uv.top_right()};
	m_vertices[lt_v] = Vertex{.position = rect.top_left(), .color = vec4_color, .uv = uv.top_left()};
}

auto IQuad::get_uv() const -> kvf::UvRect { return {.lt = m_vertices[lt_v].uv, .rb = m_vertices[rb_v].uv}; }
} // namespace le::shape
