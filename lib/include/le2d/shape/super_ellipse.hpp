#pragma once
#include "le2d/geometry.hpp"
#include "le2d/vertex_array.hpp"
#include <kvf/color.hpp>
#include <kvf/rect.hpp>

namespace le::shape {
/// \brief Super Ellipse creation parameters.
struct SuperEllipseParams {
	glm::vec2 size{200.0f};
	kvf::Color color{kvf::white_v};
	float exponent{4.0f};
	std::int32_t resolution{128};
};

/// \brief Super ellipse Geometry.
class SuperEllipse : public IGeometry {
  public:
	using Params = SuperEllipseParams;

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_verts.vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_verts.indices; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleFan; }

	void create(Params const& params = {});

	[[nodiscard]] auto get_params() const -> Params const& { return m_params; }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_params().size; }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_verts; }

  private:
	VertexArray m_verts{};
	Params m_params{};
};
} // namespace le::shape
