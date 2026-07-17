#pragma once
#include "demo/scene/scene.hpp"
#include "le2d/console/junction.hpp"
#include "le2d/console/terminal.hpp"
#include "le2d/drawable/input_text.hpp"
#include "le2d/input/mapping.hpp"
#include "le2d/tweak/tweakable.hpp"

namespace demo::scene {
class Console : public Scene {
  public:
	static constexpr std::string_view name_v{"Console"};

	explicit Console(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader);

  private:
	void tick(kvf::Seconds dt) final;
	void render_main_pass(le::IRenderer& renderer) const final;

	void load_font();
	void create_terminal();
	void create_input_text();
	void setup_tweaks();
	[[nodiscard]] auto create_mapping() -> std::shared_ptr<le::input::IMapping>;

	std::unique_ptr<le::IFont> m_mono_font{};
	std::unique_ptr<le::console::ITerminal> m_terminal{};

	std::optional<le::drawable::InputText> m_input_text{};
	std::shared_ptr<le::input::IMapping> m_mapping{};

	le::input::Router m_router{};
	std::optional<le::console::Junction> m_junction{};

	le::Tweakable<kvf::Color> m_clear_color{};
};
} // namespace demo::scene
