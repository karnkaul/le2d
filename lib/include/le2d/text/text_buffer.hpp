#pragma once
#include <le2d/primitive.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/resource/font.hpp>
#include <le2d/text/text_geometry.hpp>
#include <deque>
#include <gsl/pointers>
#include <span>

namespace le {
/// \brief Wall of text as a single Primitive.
class TextBuffer {
  public:
	explicit TextBuffer(gsl::not_null<IFontAtlas*> atlas, std::size_t limit, float n_line_spacing = 1.5f);

	void push_front(std::string text, kvf::Color color) { push_front({&text, 1}, color); }
	void push_front(std::span<std::string> lines, kvf::Color color);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_size; }

	[[nodiscard]] auto to_primitive() const -> Primitive;

  private:
	struct Line {
		std::string text{};
		kvf::Color color{};
	};

	void refresh();

	gsl::not_null<IFontAtlas*> m_atlas;
	std::size_t m_limit;
	float m_n_line_spacing;

	std::deque<Line> m_lines{};

	std::vector<kvf::ttf::GlyphLayout> m_layouts{};
	TextGeometry m_geometry{};
	glm::vec4 m_size{};
};
} // namespace le
