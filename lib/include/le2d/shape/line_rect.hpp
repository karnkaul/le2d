#pragma once
#include <kvf/rect.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/shape/shape.hpp>
#include <array>

namespace le::shape {
class LineRect {
  public:
	static constexpr std::size_t vertex_count_v{4};
	static constexpr auto indices_v = std::array{0u, 1u, 2u, 3u, 0u};

	static constexpr auto rect_v = kvf::Rect<>::from_size(glm::vec2{200.0f});

	explicit LineRect(kvf::Rect<> const& rect = rect_v, kvf::Color const color = kvf::white_v) { create(rect, color); }

	void create(kvf::Rect<> const& rect = rect_v, kvf::Color color = kvf::white_v);

	[[nodiscard]] auto get_rect() const -> kvf::Rect<>;
	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_rect().size(); }
	[[nodiscard]] auto get_origin() const -> glm::vec2 { return get_rect().center(); }
	[[nodiscard]] auto get_color() const -> kvf::Color { return get_vertices().front().color; }

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const, 4> { return m_vertices; }

	[[nodiscard]] auto get_primitive() const -> Primitive;

	ITexture const* texture{};

  private:
	std::array<Vertex, vertex_count_v> m_vertices{};
};

static_assert(ShapeT<LineRect>);
} // namespace le::shape
