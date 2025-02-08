#pragma once
#include <klib/enum_array.hpp>
#include <le2d/animation.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/input.hpp>
#include <level_info.hpp>
#include <prop.hpp>
#include <scene/scene.hpp>
#include <ui/widget.hpp>

namespace hog::scene {
class Lab : public Scene {
  public:
	explicit Lab(gsl::not_null<le::ServiceLocator*> services);

  private:
	struct TestWidget : ui::Widget {
		[[nodiscard]] auto get_hitbox() const -> kvf::Rect<> final { return hitbox.bounding_rect(); }
		void tick(kvf::Seconds /*dt*/) final {
			static constexpr auto state_colors_v = klib::EnumArray<ui::WidgetState, kvf::Color>{
				kvf::white_v,
				kvf::cyan_v,
				kvf::magenta_v,
			};
			hitbox.instance.tint = state_colors_v[get_state()];
		}
		void draw(le::Renderer& renderer) const final { hitbox.draw(renderer); }

		void on_click() final;

		le::drawable::Quad hitbox{};
	};

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

	void render_world(le::Renderer& renderer) const;
	void render_ui(le::Renderer& renderer) const;

	std::vector<le::Texture> m_textures{};
	le::drawable::Quad m_background{};
	le::drawable::Quad m_quad{};
	le::drawable::LineRect m_line_rect{};

	LevelInfo m_level_info{};

	std::vector<Prop> m_props{};

	le::input::KeyChord m_escape{};
	le::input::MouseButtonTrigger m_mb1{};

	TestWidget m_widget{};

	le::ndc::vec2 m_cursor_pos{};
	le::ndc::vec2 m_prev_cursor_pos{};
	float m_zoom_speed{0.05f};

	le::Transform m_world_view{};
};
} // namespace hog::scene
