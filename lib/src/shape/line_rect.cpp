#include <le2d/shape/line_rect.hpp>

namespace le::shape {
namespace {
constexpr std::size_t lb_v{0};
constexpr std::size_t rb_v{1};
constexpr std::size_t rt_v{2};
constexpr std::size_t lt_v{3};
} // namespace

void LineRect::create(kvf::Rect<> const& rect, kvf::Color const color) {
	m_vertices[lb_v] = Vertex{.position = rect.bottom_left(), .color = color.to_linear()};
	m_vertices[rb_v] = Vertex{.position = rect.bottom_right(), .color = color.to_linear()};
	m_vertices[rt_v] = Vertex{.position = rect.top_right(), .color = color.to_linear()};
	m_vertices[lt_v] = Vertex{.position = rect.top_left(), .color = color.to_linear()};
}

auto LineRect::get_primitive() const -> Primitive {
	return Primitive{
		.vertices = m_vertices,
		.indices = indices_v,
		.topology = vk::PrimitiveTopology::eLineStrip,
		.texture = texture,
	};
}
} // namespace le::shape
