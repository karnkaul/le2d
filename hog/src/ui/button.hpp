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

	[[nodiscard]] auto get_superellipse_params() const -> le::SuperEllipseParams const& { return m_background.get_params(); }
	void set_superellipse_params(le::SuperEllipseParams const& params) { m_background.create(params); }

	[[nodiscard]] auto get_size() const -> glm::vec2 { return m_background.get_size(); }
	void set_size(glm::vec2 size) { m_background.create(le::SuperEllipseParams{.size = size}); }

	[[nodiscard]] auto get_texture() const -> le::ITexture const* { return m_background.texture; }
	void set_texture(le::ITexture const* texture) { m_background.texture = texture; }

	[[nodiscard]] auto get_position() const -> glm::vec2 { return m_background.instance.transform.position; }
	void set_position(glm::vec2 position);

	void set_text(le::Font& font, std::string_view text);
	void set_on_click(OnClick callback) { m_on_click = std::move(callback); }

	Style style{};

  protected:
	le::drawable::SuperEllipse m_background{};
	le::drawable::Text m_text{};

	OnClick m_on_click{};
};
} // namespace hog::ui
