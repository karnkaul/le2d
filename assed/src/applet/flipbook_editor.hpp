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
	using TileEntry = imcpp::MultiSelect::Entry;

	enum class Display : std::int8_t { TileSheet, Sprite, COUNT_ };
	static constexpr auto display_str_v = klib::EnumArray<Display, std::string_view>{"TileSheet", "Sprite"};

	auto consume_scroll(event::Scroll const& scroll) -> bool final;

	void tick(kvf::Seconds dt) final;
	void render(Renderer& renderer) const final;

	void on_drop(FileDrop const& drop) final;

	void inspect();
	void inspect_display();
	void inspect_generate();
	void inspect_timeline();
	void inspect_keyframes();

	void try_load_json(FileDrop const& drop);
	void try_load_tilesheet(Uri uri);
	void try_load_animation(Uri uri);

	void generate_timeline();

	TileSheet m_tile_sheet;

	TileDrawer m_tile_drawer{};

	drawable::Sprite m_sprite{};
	anim::FlipbookAnimation m_animation{};
	anim::FlipbookAnimator m_animator{};

	struct {
		Uri tile_sheet{};
		Uri animation{};
	} m_uri{};
	Transform m_render_view{};

	float m_zoom_speed{0.1f};

	bool m_paused{};
	Display m_display{Display::TileSheet};

	struct {
		imcpp::MultiSelect select_tiles{};
		float duration{1.0f};
	} m_generate{};
};
} // namespace le::assed
