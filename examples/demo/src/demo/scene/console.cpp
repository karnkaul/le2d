#include "demo/scene/console.hpp"
#include "le2d/console/terminal_builder.hpp"
#include "le2d/error.hpp"

namespace demo::scene {
Console::Console(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader) : Scene(context, data_loader, name_v) {
	load_font();
	create_terminal();
}

void Console::tick(kvf::Seconds const dt) {
	m_junction->dispatch(get_context().event_queue());
	m_terminal->tick(dt);
}

void Console::render_main_pass(le::IRenderer& renderer) const {
	//
	m_terminal->draw(renderer);
}

void Console::load_font() {
	static constexpr std::string_view font_uri{"fonts/mono.ttf"};
	m_mono_font = get_asset_loader().load<le::IFont>(font_uri);
	if (!m_mono_font) { throw le::Error{std::format("Failed to load font: '{}'", font_uri)}; }
}

void Console::create_terminal() {
	auto builder = le::console::TerminalBuilder{};
	m_terminal = builder.build(m_mono_font.get());
	m_junction.emplace(&m_router, m_terminal.get());
}
} // namespace demo::scene
