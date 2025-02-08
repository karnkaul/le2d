#include <ui/button.hpp>

namespace hog::ui {
void Button::tick(kvf::Seconds const dt) {
	Widget::tick(dt);
	m_background.instance.tint = style.background_colors[get_state()];
	m_text.instance.tint = style.text_colors[get_state()];
}

void Button::draw(le::Renderer& renderer) const {
	m_background.draw(renderer);
	m_text.draw(renderer);
}

void Button::on_click() {
	if (m_on_click) { m_on_click(); }
}

void Button::set_size(glm::vec2 const size) { m_background.create(size); }

void Button::set_position(glm::vec2 const position) {
	m_background.instance.transform.position = m_text.instance.transform.position = position;
	m_text.instance.transform.position.y += style.text_n_y_offset * float(style.text_height);
}

void Button::set_text(le::Font& font, std::string_view const text) {
	auto const params = le::TextParams{
		.height = style.text_height,
		.expand = le::TextExpand::eBoth,
	};
	m_text.set_string(font, text, params);
}
} // namespace hog::ui
