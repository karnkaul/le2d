#pragma once
#include "le2d/geometry.hpp"
#include "le2d/vertex_array.hpp"
#include <kvf/color.hpp>
#include <kvf/rect.hpp>

namespace le::shape {
/// \brief Circle Geometry.
class Circle : public IGeometry {
  public:
	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_verts.vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_verts.indices; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleFan; }

	void create(float diameter, kvf::Color color = kvf::white_v, std::int32_t resolution = 128);

	[[nodiscard]] auto get_diameter() const -> float { return m_diameter; }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return glm::vec2{get_diameter()}; }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_verts; }

  private:
	VertexArray m_verts{};
	float m_diameter{};
};
} // namespace le::shape
