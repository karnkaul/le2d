#pragma once
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/text.hpp>
#include <ui/style.hpp>
#include <ui/widget.hpp>
#include <functional>

namespace hog::ui {
template <typename BackgroundT>
class Button : public Widget {
  public:
	using Style = ButtonStyle;
	using OnClick = std::move_only_function<void()>;

	[[nodiscard]] auto get_hitbox() const -> kvf::Rect<> final { return m_background.bounding_rect(); }

	void tick(kvf::Seconds dt) override {
		Widget::tick(dt);
		m_background.tint = style.background_colors[get_state()];
		m_text.tint = style.text_colors[get_state()];
	}

	void draw(le::Renderer& renderer) const override {
		m_background.draw(renderer);
		m_text.draw(renderer);
	}

	void on_click() override {
		if (m_on_click) { m_on_click(); }
	}

	[[nodiscard]] auto get_position() const -> glm::vec2 override { return m_background.transform.position; }

	void set_position(glm::vec2 position) override {
		m_background.transform.position = m_text.transform.position = position;
		m_text.transform.position.y += style.text_n_y_offset * float(style.text_height);
	}

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_background.get_size(); }
	virtual void set_size(glm::vec2 size) = 0;

	[[nodiscard]] auto get_texture() const -> le::ITexture const* { return m_background.texture; }
	void set_texture(le::ITexture const* texture) { m_background.texture = texture; }

	void set_text(le::Font& font, std::string_view text) {
		auto const params = le::drawable::TextParams{
			.height = style.text_height,
			.expand = le::drawable::TextExpand::eBoth,
		};
		m_text.set_string(font, text, params);
	}

	void set_on_click(OnClick callback) { m_on_click = std::move(callback); }

	Style style{};

  protected:
	BackgroundT m_background{};
	le::drawable::Text m_text{};

	OnClick m_on_click{};
};

class ButtonEllipse : public Button<le::drawable::SuperEllipse> {
  public:
	void set_size(glm::vec2 const size) final { m_background.create(le::shape::SuperEllipseParams{.size = size}); }

	[[nodiscard]] auto get_superellipse_params() const -> le::shape::SuperEllipseParams const& { return m_background.get_params(); }
	void set_superellipse_params(le::shape::SuperEllipseParams const& params) { m_background.create(params); }
};

class ButtonQuad : public Button<le::drawable::Quad> {
  public:
	void set_size(glm::vec2 const size) final { m_background.create(size); }
};
} // namespace hog::ui
