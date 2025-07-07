#pragma once
#include <applet/applet.hpp>
#include <tile_drawer.hpp>

namespace le::assed {
class TileSheetEditor : public Applet {
  public:
	static constexpr klib::CString name_v{"TileSheet Editor"};

	[[nodiscard]] auto get_name() const -> klib::CString final { return name_v; }

	explicit TileSheetEditor(gsl::not_null<ServiceLocator const*> services);

  private:
	auto consume_cursor_move(glm::vec2 cursor) -> bool final;
	auto consume_mouse_button(event::MouseButton const& button) -> bool final;

	void tick(kvf::Seconds dt) final;
	void render(Renderer& renderer) const final;

	void on_drop(FileDrop const& drop) final;

	void populate_file_menu() final;

	void inspect();
	void inspect_selected();

	void try_load_json(FileDrop const& drop);
	void try_load_tilesheet(Uri uri);
	void try_load_tileset(Uri const& uri);
	void try_load_texture(Uri uri);

	void set_tiles(std::span<Tile const> tiles);
	void set_texture(std::unique_ptr<ITexture> texture);

	void generate_tiles();
	void set_unsaved();

	void on_click();
	void on_save();

	std::unique_ptr<ITexture> m_texture{};
	TileDrawer m_drawer{};
	std::vector<Tile> m_tiles{};
	glm::vec2 m_cursor_pos{};

	struct {
		Uri texture{};
		Uri tile_sheet{};
	} m_uri{};

	glm::ivec2 m_split_dims{1, 1};
};
} // namespace le::assed
