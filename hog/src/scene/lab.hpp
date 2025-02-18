#pragma once
#include <game/level.hpp>
#include <game/level_info.hpp>
#include <game/sidebar.hpp>
#include <klib/enum_array.hpp>
#include <le2d/animation.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/input/controls.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
class Lab : public Scene {
  public:
	explicit Lab(gsl::not_null<le::ServiceLocator*> services);

  private:
	auto consume_cursor_move(glm::vec2 pos) -> bool override;
	auto consume_key(le::event::Key const& key) -> bool override;
	auto consume_mouse_button(le::event::MouseButton const& button) -> bool override;
	auto consume_scroll(le::event::Scroll const& scroll) -> bool override;

	[[nodiscard]] auto clear_color() const -> kvf::Color final { return m_level_info.background.color; }

	void tick(kvf::Seconds dt) final;
	void render(le::Renderer& renderer) const final;
	void disengage_input() final;

	void load_assets();

	void check_hit(glm::vec2 cursor_pos);
	void collect(std::size_t collectible_index);

	void inspect();
	void inspect_collectibles();

	void render_world(le::Renderer& renderer) const;
	void render_ui(le::Renderer& renderer) const;

	std::vector<le::Texture> m_textures{};
	le::drawable::Quad m_background{};

	LevelInfo m_level_info{};
	Level m_level{};

	Sidebar m_sidebar;

	le::input::KeyChord m_escape{};
	le::input::MouseButtonTrigger m_drag_view{};
	le::input::MouseButtonChord m_click{};
	bool m_check_hit{};

	glm::vec2 m_cursor_pos{};
	glm::vec2 m_prev_cursor_pos{};
	float m_zoom_speed{0.05f};

	le::Transform m_world_view{};
};
} // namespace hog::scene
