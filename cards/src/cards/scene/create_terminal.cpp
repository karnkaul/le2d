#include "cards/scene/create_terminal.hpp"
#include "cards/log.hpp"
#include "le2d/console/terminal_builder.hpp"

namespace cards::scene {
CreateTerminal::CreateTerminal(gsl::not_null<Console*> console, gsl::not_null<Catalog*> catalog, std::string_view next_scene)
	: m_console(console), m_catalog(catalog), m_next_scene(next_scene) {}

void CreateTerminal::initialize() {
	auto const mono_font = m_catalog->get_mono_font();
	if (!mono_font) { return; }

	log.info("mono font found, creating terminal");
	m_console->terminal = le::console::TerminalBuilder{}.build(mono_font);
	get_coordinator().get_input_router().terminal_mapping = m_console->terminal->get_mapping();

	m_console->timing.set_value(std::string{timing_type_name_map.to_name(m_catalog->get_timing_type())});
	m_console->timing.on_set([catalog = m_catalog](std::string_view const value) {
		auto const timing_type = timing_type_name_map.to_enum(value);
		if (!timing_type) { return false; }
		catalog->set_timing_type(*timing_type);
		return true;
	});
	m_console->terminal->add_tweakable("timing", &m_console->timing);
}

void CreateTerminal::tick(kvf::Seconds const /*dt*/) { get_coordinator().enqueue_scene(m_next_scene); }
} // namespace cards::scene
