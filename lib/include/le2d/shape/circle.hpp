#pragma once
#include <kvf/color.hpp>
#include <kvf/rect.hpp>
#include <le2d/geometry.hpp>
#include <le2d/vertex_array.hpp>

namespace le::shape {
struct CircleParams {
	float diameter{200.0f};
	kvf::Color color{kvf::white_v};
	std::int32_t resolution{128};
};

class Circle : public IGeometry {
  public:
	using Params = CircleParams;

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_verts.vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_verts.indices; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleFan; }

	void create(Params const& params = {});
	void create(float const diameter = 200.0f) { create(Params{.diameter = diameter}); }

	[[nodiscard]] auto get_params() const -> Params const& { return m_params; }
	[[nodiscard]] auto get_diameter() const -> float { return get_params().diameter; }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return glm::vec2{get_diameter()}; }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_verts; }

  private:
	VertexArray m_verts{};
	Params m_params{};
};
} // namespace le::shape
