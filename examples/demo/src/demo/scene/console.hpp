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
	explicit TextBox(gsl::not_null<le::IFont*> font);

	void draw(le::IRenderer& renderer) const final;
	void tick(kvf::Seconds dt);

	[[nodiscard]] auto get_mapping() const -> std::shared_ptr<le::input::IMapping> const& { return m_mapping; }
	void set_interactive(bool const interactive) { m_input_text.set_interactive(interactive); }

  private:
	void create_quads();
	void create_mapping();

	le::drawable::Quad m_background{};
	le::drawable::Quad m_baseline{};
	le::drawable::InputText m_input_text;
	std::shared_ptr<le::input::IMapping> m_mapping{};
};

class Console : public Scene {
  public:
	static constexpr std::string_view name_v{"Console"};

	explicit Console(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader);

  private:
	void tick(kvf::Seconds dt) final;
	void render_main_pass(le::IRenderer& renderer) const final;

	void load_font();
	void create_terminal();
	void create_text_box();
	void setup_tweaks();

	std::unique_ptr<le::IFont> m_mono_font{};
	std::unique_ptr<le::console::ITerminal> m_terminal{};

	le::input::Router m_router{};
	std::optional<le::console::Junction> m_junction{};

	std::optional<TextBox> m_text_box{};

	le::Tweakable<kvf::Color> m_clear_color{};
};
} // namespace demo::scene
