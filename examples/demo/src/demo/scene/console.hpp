#pragma once
#include "demo/scene/scene.hpp"
#include "le2d/console/junction.hpp"
#include "le2d/console/terminal.hpp"

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

	std::unique_ptr<le::IFont> m_mono_font{};
	std::unique_ptr<le::console::ITerminal> m_terminal{};

	le::input::Router m_router{};
	std::optional<le::console::Junction> m_junction{};
};
} // namespace demo::scene
