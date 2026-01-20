#pragma once
#include "le2d/shape/sector.hpp"

namespace le::shape {
/// \brief Circle creation parameters.
struct CircleParams {
	kvf::Color color{kvf::white_v};
	std::int32_t resolution{128};
};

/// \brief Circle Geometry.
class Circle : public IGeometry {
  public:
	using Params = CircleParams;

	static constexpr auto default_diameter_v = default_length_v;

	explicit(false) Circle(float const diameter = default_diameter_v, CircleParams const& params = {}) { create(diameter, params); }

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_sector.get_vertices(); }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_sector.get_indices(); }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return m_sector.get_topology(); }

	void create(float diameter = default_diameter_v, Params const& params = {});

	[[nodiscard]] auto get_diameter() const -> float { return m_sector.get_diameter(); }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return glm::vec2{get_diameter()}; }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_sector.get_vertex_array(); }

  private:
	Sector m_sector{};
};
} // namespace le::shape
