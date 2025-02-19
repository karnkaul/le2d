#pragma once
#include <le2d/drawable/drawable.hpp>
#include <le2d/font.hpp>
#include <le2d/geometry.hpp>

namespace le::drawable {
enum class TextExpand : std::int8_t { eBoth, eRight, eLeft };

struct TextParams {
	TextHeight height{TextHeight::Default};
	TextExpand expand{TextExpand::eBoth};
};

class TextGeometry : public IGeometry {
  public:
	using Params = TextParams;

	[[nodiscard]] auto get_vertices() const -> std::span<Vertex const> final { return m_verts.vertices; }
	[[nodiscard]] auto get_indices() const -> std::span<std::uint32_t const> final { return m_verts.indices; }
	[[nodiscard]] auto get_topology() const -> vk::PrimitiveTopology final { return vk::PrimitiveTopology::eTriangleList; }

	void set_string(FontAtlas const& font_atlas, std::string_view line, TextExpand expand);

	[[nodiscard]] auto get_glyph_layouts() const -> std::span<kvf::ttf::GlyphLayout const> { return m_glyph_layouts; }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_verts; }

  private:
	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	VertexArray m_verts{};
	glm::vec2 m_size{};
};

class TextBase : public IDrawable {
  public:
	using Params = TextParams;

	[[nodiscard]] virtual auto get_instances() const -> std::span<RenderInstance const> = 0;

	void set_string(Font& font, std::string_view line, Params const& params = {});

	[[nodiscard]] auto get_primitive() const -> Primitive;
	[[nodiscard]] auto get_texture() const -> Texture const* { return m_texture; }

	void draw(Renderer& renderer) const override { renderer.draw(get_primitive(), get_instances()); }

  private:
	TextGeometry m_geometry{};
	Texture const* m_texture{};
};

class Text : public TextBase, public RenderInstance {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return {static_cast<RenderInstance const*>(this), 1}; }
};

class InstancedText : public TextBase {
  public:
	[[nodiscard]] auto get_instances() const -> std::span<RenderInstance const> final { return instances; }

	std::vector<RenderInstance> instances{};
};
} // namespace le::drawable
