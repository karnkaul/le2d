#pragma once
#include <kvf/color.hpp>
#include <kvf/rect.hpp>
#include <le2d/geometry.hpp>
#include <le2d/vertex_array.hpp>

namespace le::shape {
class IQuad : public IGeometry {
  public:
	static constexpr std::size_t vertex_count_v{4};
	static constexpr auto size_v = glm::vec2{200.0f};

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_vertices; }

	void create(glm::vec2 const size = size_v) { create(kvf::Rect<>::from_size(size)); }

	void create(kvf::Rect<> const& rect, kvf::UvRect const& uv = kvf::uv_rect_v, kvf::Color color = kvf::white_v);

	[[nodiscard]] auto get_rect() const -> kvf::Rect<>;
	[[nodiscard]] auto get_uv() const -> kvf::UvRect;
	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_rect().size(); }
	[[nodiscard]] auto get_origin() const -> glm::vec2 { return get_rect().center(); }

  private:
	std::array<Vertex, vertex_count_v> m_vertices{};
};

class Quad : public IQuad {
  public:
	static constexpr auto indices_v = std::array{0u, 1u, 2u, 2u, 3u, 0u};

	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return indices_v; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleList; }
};

class LineRect : public IQuad {
  public:
	static constexpr auto indices_v = std::array{0u, 1u, 2u, 3u, 0u};

	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return indices_v; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eLineStrip; }
};
} // namespace le::shape
