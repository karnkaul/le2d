#pragma once
#include "demo/scene/scene.hpp"
#include "le2d/console/junction.hpp"
#include "le2d/console/terminal.hpp"
#include "le2d/drawable/input_text.hpp"
#include "le2d/drawable/shape.hpp"
#include "le2d/input/mapping.hpp"
#include "le2d/tweak/tweakable.hpp"

namespace demo::scene {
class TextBox : public le::IDrawable {
  public:
	explicit TextBox(gsl::not_null<le::IFont*> font, gsl::not_null<le::input::Router*> router);

	void draw(le::IRenderer& renderer) const final;
	void tick(kvf::Seconds dt);

	void activate();
	void deactivate();

  private:
	void create_quads();
	void create_mapping();

	gsl::not_null<le::input::Router*> m_router;

	le::drawable::Quad m_background{};
	le::drawable::Quad m_baseline{};
	le::drawable::InputText m_input_text;
	std::shared_ptr<le::input::IMapping> m_mapping{};
};

class TextBoxConsole : public Scene {
  public:
	static constexpr std::string_view name_v{"TextBox & Console"};

	explicit TextBoxConsole(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader);

  private:
	void tick(kvf::Seconds dt) final;
	void render_main_pass(le::IRenderer& renderer) const final;

	void load_font();
	void create_terminal();
	void create_mapping();
	void create_text_box();
	void setup_tweaks();

	std::unique_ptr<le::IFont> m_mono_font{};
	std::unique_ptr<le::console::ITerminal> m_terminal{};

	le::input::Router m_router{};
	std::optional<le::console::Junction> m_junction{};

	std::shared_ptr<le::input::IMapping> m_mapping{};
	std::optional<TextBox> m_text_box{};

	le::Tweakable<kvf::Color> m_clear_color{};
};
} // namespace demo::scene
