#pragma once
#include <kvf/rect.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/shapes/shape.hpp>
#include <array>
#include <memory>

namespace le::shape {
class Quad {
  public:
	static constexpr auto indices_v = std::array{0u, 1u, 2u, 2u, 3u, 0u};

	static constexpr auto rect_v = kvf::Rect<>::from_size(glm::vec2{200.0f});

	explicit Quad(kvf::Rect<> const& rect = rect_v, kvf::UvRect const& uv = kvf::uv_rect_v) { set_rect(rect, uv); }

	void set_rect(kvf::Rect<> const& rect, kvf::UvRect const& uv = kvf::uv_rect_v);

	[[nodiscard]] auto get_rect() const -> kvf::Rect<>;
	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_rect().size(); }
	[[nodiscard]] auto get_origin() const -> glm::vec2 { return get_rect().center(); }
	[[nodiscard]] auto get_uv() const -> kvf::UvRect;

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const, 4> { return m_vertices; }

	[[nodiscard]] auto get_primitive() const -> Primitive;

	std::shared_ptr<ITexture const> texture{};

  private:
	std::array<Vertex, 4> m_vertices{};
};

static_assert(ShapeT<Quad>);
} // namespace le::shape
