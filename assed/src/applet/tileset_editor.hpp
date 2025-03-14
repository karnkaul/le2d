#pragma once
#include <applet/applet.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/texture.hpp>
#include <optional>

namespace le::assed {
class TilesetEditor : public Applet {
  public:
	explicit TilesetEditor(gsl::not_null<ServiceLocator const*> services);

  private:
	auto consume_cursor_move(glm::vec2 cursor) -> bool final;
	auto consume_mouse_button(event::MouseButton const& button) -> bool final;
	auto consume_scroll(event::Scroll const& scroll) -> bool final;
	auto consume_drop(event::Drop const& drop) -> bool final;

	void tick(kvf::Seconds dt) final;
	void render(Renderer& renderer) const final;

	void inspect();

	void try_load_tileset(Uri uri);
	void try_load_texture(Uri uri);

	void setup_tile_frames();
	void generate_tiles();

	void on_click();

	Texture m_texture;
	drawable::Quad m_quad{};
	std::vector<Tile> m_tiles{};
	std::vector<drawable::LineRect> m_tile_frames{};
	std::optional<std::size_t> m_selected_tile{};
	glm::vec2 m_cursor_pos{};

	Uri m_loaded_uri{};
	Transform m_render_view{};

	float m_zoom_speed{0.1f};

	glm::ivec2 m_split_dims{1, 1};
};
} // namespace le::assed
