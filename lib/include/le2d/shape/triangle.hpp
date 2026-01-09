#pragma once
#include "le2d/geometry.hpp"
#include <kvf/color.hpp>
#include <kvf/rect.hpp>
#include <array>

namespace le::shape {
/// \brief Triangle Geometry.
class Triangle : public IGeometry {
  public:
	static constexpr auto vertex_count_v{4uz};

	static constexpr auto default_radius_v = 0.5f * default_length_v;

	explicit(false) Triangle(float const radius = default_radius_v, kvf::Color const color = kvf::white_v) { create(radius, color); }

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return {}; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleList; }

	void create(float radius = default_radius_v, kvf::Color color = kvf::white_v);

	std::array<Vertex, vertex_count_v> vertices{};
};
} // namespace le::shape
