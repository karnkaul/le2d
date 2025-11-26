#pragma once
#include "le2d/geometry.hpp"
#include <kvf/color.hpp>
#include <kvf/rect.hpp>
#include <array>

namespace le::shape {
/// \brief Triangle Geometry.
class Triangle : public IGeometry {
  public:
	static constexpr std::size_t vertex_count_v{4};

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return {}; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleList; }

	void create(float radius = 100.0f, kvf::Color color = kvf::white_v);

	std::array<Vertex, vertex_count_v> vertices{};
};
} // namespace le::shape
