#pragma once
#include <applet/applet.hpp>
#include <klib/enum_array.hpp>
#include <le2d/anim/animator.hpp>
#include <le2d/drawable/sprite.hpp>
#include <tile_drawer.hpp>

namespace le::assed {
class FlipbookEditor : public Applet {
  public:
	static constexpr klib::CString name_v{"Flipbook Editor"};

	[[nodiscard]] auto get_name() const -> klib::CString final { return name_v; }

	explicit FlipbookEditor(gsl::not_null<ServiceLocator const*> services);

  private:
	enum class Display : std::int8_t { TileSheet, Sprite, COUNT_ };
	static constexpr auto display_str_v = klib::EnumArray<Display, std::string_view>{"TileSheet", "Sprite"};

	void tick(kvf::Seconds dt) final;
	void render(IRenderer& renderer) const final;

	void on_drop(FileDrop const& drop) final;

	void populate_file_menu() final;

	void inspect();
	void inspect_display();
	void inspect_generate();
	void inspect_timeline();
	void inspect_keyframes();

	void try_load_json(FileDrop const& drop);
	void try_load_tilesheet(Uri uri);
	void try_load_animation(Uri uri);
	void on_save();

	void generate_timeline();

	std::unique_ptr<ITileSheet> m_tile_sheet{};

	TileDrawer m_drawer{};

	drawable::Sprite m_sprite{};
	FlipbookAnimation m_animation{};
	FlipbookAnimator m_animator{};

	struct {
		Uri tile_sheet{};
		Uri animation{};
	} m_uri{};

	bool m_paused{};
	Display m_display{Display::TileSheet};

	struct {
		imcpp::MultiSelect select_tiles{};
		float duration{1.0f};
	} m_generate{};
};
} // namespace le::assed
