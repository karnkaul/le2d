#include "demo/scene/console.hpp"
#include "le2d/console/terminal_builder.hpp"
#include "le2d/error.hpp"
#include "le2d/input/listener_mapping.hpp"

namespace demo::scene {
Console::Console(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader) : Scene(context, data_loader, name_v) {
	load_font();
	create_terminal();
	create_input_text();
	setup_tweaks();
}

void Console::tick(kvf::Seconds const dt) {
	m_junction->dispatch(get_context().event_queue());
	m_terminal->tick(dt);
	m_input_text->tick(dt);
}

void Console::render_main_pass(le::IRenderer& renderer) const {
	m_input_text->draw(renderer);
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

void Console::create_input_text() {
	m_input_text.emplace(m_mono_font.get());
	m_mapping = create_mapping();
	m_router.push_mapping(m_mapping);
	m_input_text->set_interactive(true);
}

void Console::setup_tweaks() {
	m_clear_color.set_value(kvf::Color{0x113377ff});
	m_main_pass_clear = m_clear_color.get_value();

	m_clear_color.on_set([this](kvf::Color const& clear) -> bool {
		m_main_pass_clear = clear;
		return true;
	});
	m_terminal->add_tweakable("clear.color", &m_clear_color);
}

auto Console::create_mapping() -> std::shared_ptr<le::input::IMapping> {
	auto ret = std::make_shared<le::input::ListenerMapping>();
	ret->on_key = [this](le::event::Key const& key) { m_input_text->on_key(key); };
	ret->on_codepoint = [this](le::event::Codepoint const codepoint) { m_input_text->on_codepoint(codepoint); };
	return ret;
}
} // namespace demo::scene
