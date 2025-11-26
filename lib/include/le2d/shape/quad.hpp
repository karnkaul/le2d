#pragma once
#include "le2d/geometry.hpp"
#include <kvf/color.hpp>
#include <kvf/rect.hpp>

namespace le::shape {
/// \brief Interface for Quad Geometry.
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

/// \brief Quad Geometry.
class Quad : public IQuad {
  public:
	static constexpr auto indices_v = std::array{0u, 1u, 2u, 2u, 3u, 0u};

	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return indices_v; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleList; }
};

/// \brief Line rectangle Geometry. (Quad outline.)
class LineRect : public IQuad {
  public:
	static constexpr auto indices_v = std::array{0u, 1u, 2u, 3u, 0u};

	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return indices_v; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eLineStrip; }
};
} // namespace le::shape
