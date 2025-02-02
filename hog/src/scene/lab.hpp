#pragma once
#include <le2d/drawable/shape.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
class Lab : public Scene {
  public:
	explicit Lab(gsl::not_null<le::ServiceLocator*> services);

  private:
	struct HeldKeys {
		bool a{};
		bool d{};
		bool q{};
		bool e{};
	};

	void on_event(le::event::Key key) final;
	void on_event(le::event::CursorPos pos) final;
	void on_event(le::event::Scroll scroll) final;

	[[nodiscard]] auto clear_color() const -> kvf::Color final { return kvf::Color{0x222222ff}; }

	void tick(kvf::Seconds dt) final;
	void render(le::Renderer& renderer) const final;
	void reset_events() final;

	void load_fonts();
	void create_textures();

	std::vector<le::Texture> m_textures{};
	le::drawable::Quad m_quad{};
	le::drawable::LineRect m_line_rect{};

	HeldKeys m_held_keys{};
	le::ndc::vec2 m_cursor_pos{};
	float m_zoom_speed{0.05f};

	le::Transform m_render_view{};
};
} // namespace hog::scene
