#pragma once
#include <kvf/ttf.hpp>
#include <le2d/geometry.hpp>
#include <le2d/primitive.hpp>
#include <le2d/vertex_array.hpp>

namespace le {
class TextGeometry : public IGeometry {
  public:
	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_vertices.vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_vertices.indices; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleList; }

	void append_glyphs(std::span<kvf::ttf::GlyphLayout const> layouts, glm::vec2 offset = {}, kvf::Color color = kvf::white_v);
	void clear_vertices() { m_vertices.clear(); }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_vertices; }
	[[nodiscard]] auto to_primitive(Texture const& font_atlas) const -> Primitive;

  private:
	VertexArray m_vertices{};
};
} // namespace le
