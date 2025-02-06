#pragma once
#include <le2d/animation.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/input.hpp>
#include <level_info.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
class Lab : public Scene {
  public:
	explicit Lab(gsl::not_null<le::ServiceLocator*> services);

  private:
	void on_event(le::event::Key key) final;
	void on_event(le::event::MouseButton button) final;
	void on_event(le::event::CursorPos pos) final;
	void on_event(le::event::Scroll scroll) final;

	[[nodiscard]] auto clear_color() const -> kvf::Color final { return m_level_info.background.color; }

	void tick(kvf::Seconds dt) final;
	void render(le::Renderer& renderer) const final;
	void disengage_input() final;

	void load_assets();
	void create_textures();

	void inspect();

	std::vector<le::Texture> m_textures{};
	le::drawable::Quad m_background{};
	le::drawable::Quad m_quad{};
	le::drawable::LineRect m_line_rect{};
	le::Animator<le::Animation> m_anim{};

	LevelInfo m_level_info{};

	le::input::KeyAxis m_horz{};
	le::input::KeyAxis m_rotate{};
	le::input::KeyChord m_escape{};
	le::input::MouseButtonTrigger m_mb1{};

	le::ndc::vec2 m_cursor_pos{};
	le::ndc::vec2 m_prev_cursor_pos{};
	float m_translate_speed{500.0f};
	float m_zoom_speed{0.05f};

	le::Transform m_render_view{};
};
} // namespace hog::scene
