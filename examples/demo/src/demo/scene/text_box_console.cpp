#include "demo/scene/text_box_console.hpp"
#include "le2d/console/terminal_builder.hpp"
#include "le2d/error.hpp"
#include "le2d/input/listener_mapping.hpp"

namespace demo::scene {

TextBox::TextBox(gsl::not_null<le::IFont*> font, gsl::not_null<le::input::Router*> router) : m_router(router), m_input_text(font) {
	create_quads();
	create_mapping();
}

void TextBox::draw(le::IRenderer& renderer) const {
	m_background.draw(renderer);
	m_baseline.draw(renderer);
	auto const scissor = renderer.scissor_rect;
	renderer.scissor_rect = renderer.world_to_scissor(m_background.bounding_rect());
	m_input_text.draw(renderer);
	renderer.scissor_rect = scissor;
}

void TextBox::tick(kvf::Seconds const dt) {
	m_input_text.tick(dt);
	m_background.instance.tint = m_input_text.is_interactive() ? kvf::Color{0x772222ff} : kvf::Color{0x222222ff};
}

void TextBox::create_quads() {
	m_baseline.geometry.create({400.0f, 1.0f});
	m_baseline.instance.tint = kvf::Color{0x777777ff};

	m_background.geometry.create({m_baseline.geometry.get_size().x + 40.0f, 70.0f});
	m_background.instance.transform.position.y = (0.5f * m_background.geometry.get_size().y) - 20.0f;

	m_input_text.instance.transform.position.x = (-0.5f * m_baseline.geometry.get_size().x) + 2.0f;
}

void TextBox::create_mapping() {
	auto mapping = std::make_shared<le::input::ListenerMapping>();
	mapping->on_key = [this](le::event::Key const& key) {
		if (key.mods == 0 && key.action == GLFW_PRESS && key.key == GLFW_KEY_ESCAPE) { deactivate(); }
		m_input_text.on_key(key);
	};
	mapping->on_codepoint = [this](le::event::Codepoint const codepoint) { m_input_text.on_codepoint(codepoint); };
	m_mapping = std::move(mapping);
}

void TextBox::activate() {
	m_router->push_mapping(m_mapping);
	m_input_text.set_interactive(true);
}

void TextBox::deactivate() {
	m_router->remove_mapping(m_mapping);
	m_input_text.set_interactive(false);
}

TextBoxConsole::TextBoxConsole(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader)
	: Scene(context, data_loader, name_v) {
	load_font();
	create_terminal();
	create_text_box();
	setup_tweaks();
}

void TextBoxConsole::tick(kvf::Seconds const dt) {
	m_junction->dispatch(get_context().event_queue());
	m_terminal->tick(dt);
	m_text_box->tick(dt);
}

void TextBoxConsole::render_main_pass(le::IRenderer& renderer) const {
	m_text_box->draw(renderer);
	m_terminal->draw(renderer);
}

void TextBoxConsole::load_font() {
	static constexpr std::string_view font_uri{"fonts/mono.ttf"};
	m_mono_font = get_asset_loader().load<le::IFont>(font_uri);
	if (!m_mono_font) { throw le::Error{std::format("Failed to load font: '{}'", font_uri)}; }
}

void TextBoxConsole::create_terminal() {
	auto builder = le::console::TerminalBuilder{};
	m_terminal = builder.build(m_mono_font.get());
	m_junction.emplace(&m_router, m_terminal.get());
}

void TextBoxConsole::create_text_box() {
	auto mapping = std::make_shared<le::input::ListenerMapping>();
	mapping->on_key = [this](le::event::Key const& key) {
		if (key.mods != 0 || key.action != GLFW_PRESS || key.key != GLFW_KEY_ENTER) { return; }
		m_text_box->activate();
	};
	m_mapping = std::move(mapping);
	m_router.push_mapping(m_mapping);

	m_text_box.emplace(m_mono_font.get(), &m_router);
	m_text_box->activate();
}

void TextBoxConsole::setup_tweaks() {
	m_clear_color.set_value(kvf::Color{0x113377ff});
	m_main_pass_clear = m_clear_color.get_value();

	m_clear_color.on_set([this](kvf::Color const& clear) -> bool {
		m_main_pass_clear = clear;
		return true;
	});
	m_terminal->add_tweakable("clear.color", &m_clear_color);
}
} // namespace demo::scene
