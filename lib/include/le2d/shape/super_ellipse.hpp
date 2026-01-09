#pragma once
#include "le2d/geometry.hpp"
#include "le2d/vertex_array.hpp"
#include <kvf/color.hpp>
#include <kvf/rect.hpp>

namespace le::shape {
/// \brief Super Ellipse creation parameters.
struct SuperEllipseParams {
	kvf::Color color{kvf::white_v};
	float exponent{4.0f};
	std::int32_t resolution{128};
};

/// \brief Super ellipse Geometry.
class SuperEllipse : public IGeometry {
  public:
	using Params = SuperEllipseParams;

	static constexpr auto default_size_v = glm::vec2{default_length_v};

	explicit(false) SuperEllipse(glm::vec2 const size = default_size_v, Params const& params = {}) { create(size, params); }

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_verts.vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_verts.indices; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleFan; }

	void create(glm::vec2 size = default_size_v, Params const& params = {});

	[[nodiscard]] auto get_params() const -> Params const& { return m_params; }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_verts; }

  private:
	VertexArray m_verts{};
	glm::vec2 m_size{};
	Params m_params{};
};
} // namespace le::shape
