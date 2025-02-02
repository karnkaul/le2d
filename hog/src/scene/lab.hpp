#pragma once
#include <le2d/drawable/shape.hpp>
#include <le2d/input.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
class Lab : public Scene {
  public:
	explicit Lab(gsl::not_null<le::ServiceLocator*> services);

  private:
	void on_event(le::event::Key key) final;
	void on_event(le::event::CursorPos pos) final;
	void on_event(le::event::Scroll scroll) final;

	[[nodiscard]] auto clear_color() const -> kvf::Color final { return kvf::Color{0x222222ff}; }

	void tick(kvf::Seconds dt) final;
	void render(le::Renderer& renderer) const final;
	void disengage_input() final;

	void load_fonts();
	void create_textures();

	void inspect();

	std::vector<le::Texture> m_textures{};
	le::drawable::Quad m_quad{};
	le::drawable::LineRect m_line_rect{};

	le::input::KeyAxis m_horz{};
	le::input::KeyAxis m_rotate{};
	le::input::KeyTrigger m_escape{};

	le::ndc::vec2 m_cursor_pos{};
	float m_translate_speed{500.0f};
	float m_zoom_speed{0.05f};

	le::Transform m_render_view{};
};
} // namespace hog::scene
