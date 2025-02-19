#pragma once
#include <le2d/font.hpp>
#include <le2d/geometry.hpp>
#include <le2d/primitive.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/vertex_array.hpp>
#include <deque>
#include <gsl/pointers>
#include <span>

namespace le {
class TextBuffer : public IGeometry {
  public:
	explicit TextBuffer(gsl::not_null<FontAtlas*> atlas, std::size_t limit, float n_line_spacing = 1.5f);

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_verts.vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_verts.indices; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleList; }

	void push_front(std::string text, kvf::Color color) { push_front({&text, 1}, color); }
	void push_front(std::span<std::string> lines, kvf::Color color);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }

	[[nodiscard]] auto get_primitive() const -> Primitive;

  private:
	struct Line {
		std::string text{};
		kvf::Color color{};
	};

	void refresh();

	gsl::not_null<FontAtlas*> m_atlas;
	std::size_t m_limit;
	float m_n_line_spacing;

	std::deque<Line> m_lines{};

	std::vector<kvf::ttf::GlyphLayout> m_layouts{};
	VertexArray m_verts{};
	glm::vec4 m_size{};
};
} // namespace le
