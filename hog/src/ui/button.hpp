#pragma once
#include <le2d/drawable/shape.hpp>
#include <ui/style.hpp>
#include <ui/widget.hpp>
#include <functional>

namespace hog::ui {
class Button : public Widget {
  public:
	using Style = ButtonStyle;
	using OnClick = std::move_only_function<void()>;

	[[nodiscard]] auto get_hitbox() const -> kvf::Rect<> final { return m_background.bounding_rect(); }

	void tick(kvf::Seconds dt) override;
	void draw(le::Renderer& renderer) const override;

	void on_click() override;

	void set_size(glm::vec2 size);
	void set_position(glm::vec2 position);
	void set_text(le::Font& font, std::string_view text);
	void set_on_click(OnClick callback) { m_on_click = std::move(callback); }

	Style style{};

  protected:
	le::drawable::Quad m_background{};
	le::drawable::Text m_text{};

	OnClick m_on_click{};
};
} // namespace hog::ui
