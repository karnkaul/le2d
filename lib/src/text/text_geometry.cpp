#include <le2d/font.hpp>
#include <le2d/text/text_geometry.hpp>

namespace le {
void TextGeometry::append_glyphs(std::span<kvf::ttf::GlyphLayout const> layouts, glm::vec2 const offset, kvf::Color const color) {
	write_glyphs(m_vertices, layouts, offset, color);
}

auto TextGeometry::to_primitive(Texture const& font_atlas) const -> Primitive {
	return Primitive{
		.vertices = m_vertices.vertices,
		.indices = m_vertices.indices,
		.topology = vk::PrimitiveTopology::eTriangleList,
		.texture = &font_atlas,
	};
}
} // namespace le
