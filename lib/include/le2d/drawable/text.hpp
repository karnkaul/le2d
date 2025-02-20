#pragma once
#include <le2d/drawable/drawable.hpp>
#include <le2d/font.hpp>
#include <le2d/text/text_geometry.hpp>

namespace le::drawable {
enum class TextExpand : std::int8_t { eBoth, eRight, eLeft };

struct TextParams {
	TextHeight height{TextHeight::Default};
	TextExpand expand{TextExpand::eBoth};
};

class TextBase : public IDrawable {
  public:
	using Params = TextParams;

	[[nodiscard]] virtual auto get_instances() const -> std::span<RenderInstance const> = 0;

	void set_string(Font& font, std::string_view line, Params const& params = {});

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }
	[[nodiscard]] auto get_texture() const -> Texture const* { return m_texture; }
	[[nodiscard]] auto to_primitive() const -> Primitive;

	void draw(Renderer& renderer) const override { renderer.draw(to_primitive(), get_instances()); }

  private:
	TextGeometry m_geometry{};
	std::vector<kvf::ttf::GlyphLayout> m_glyph_layouts{};
	Texture const* m_texture{};
	glm::vec2 m_size{};
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
