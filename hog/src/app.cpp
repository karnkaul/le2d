#include <app.hpp>
#include <klib/visitor.hpp>
#include <le2d/asset/loaders.hpp>
#include <log.hpp>
#include <scene/lab.hpp>

namespace hog {
namespace {
auto const context_ci = le::ContextCreateInfo{
	.window = le::WindowInfo{.size = {1280, 720}, .title = "le2d example"},
	// .window = FullscreenInfo{.title = "le2d example"},
	.default_shader = {.vertex = "default.vert", .fragment = "default.frag"},
	.framebuffer_samples = vk::SampleCountFlagBits::e2,
};
}

App::App(gsl::not_null<le::IDataLoader const*> data_loader) : m_context(data_loader, context_ci) {}

void App::run() {
	m_blocker = m_context.create_device_block();

	auto font_loader = le::asset::FontLoader{&m_context};
	auto main_font = font_loader.load("fonts/main.ttf");
	if (!main_font) { throw std::runtime_error{"Failed to load main font"}; }
	m_resources.emplace(std::move(main_font->asset));

	if (auto font = font_loader.load("fonts/mono.ttf")) {
		m_terminal.emplace(&font->asset, m_context.framebuffer_size());
		m_terminal->add_command("quit", {}, [&](le::console::Printer& printer) {
			printer.println("quitting");
			m_context.shutdown();
		});
		m_resources->insert_base("mono.ttf", std::move(font));
	}

	m_services.bind(&m_context);
	m_services.bind(&m_context.get_audio());
	m_services.bind(&*m_resources);
	m_services.bind(&m_input_dispatch);
	m_services.bind<scene::ISwitcher>(this);

	m_scene = std::make_unique<scene::Lab>(&m_services);

	m_delta_time.reset();

	while (m_context.is_running()) {
		m_context.next_frame();

		auto const dt = m_delta_time.tick();
		tick(dt);
		if (auto renderer = m_context.begin_render(m_scene->clear_color())) { render(renderer); }

		m_context.present();
	}
}

void App::enqueue_switch(SwitchFunc create_scene) { m_create_scene = std::move(create_scene); }

void App::tick(kvf::Seconds const dt) {
	process_events();

	if (m_create_scene) {
		auto next_scene = m_create_scene();
		if (next_scene) {
			m_context.get_render_window().get_render_device().get_device().waitIdle();
			m_scene = std::move(next_scene);
			m_delta_time.reset();
		}
		m_create_scene = {};
	}

	m_scene->tick(dt);

	if (m_terminal) { m_terminal->tick(m_context.framebuffer_size(), dt); }
}

void App::render(le::Renderer& renderer) const {
	m_scene->render(renderer);

	if (m_terminal) { m_terminal->draw(renderer); }
}

void App::process_events() {
	auto const events = m_context.event_queue();

	auto terminal_activated = false;
	if (m_terminal) { m_terminal->handle_events(events, &terminal_activated); }
	if (terminal_activated) { m_scene->disengage_input(); }

	if (m_terminal && !m_terminal->is_active()) {
		auto const visitor = klib::SubVisitor{
			[this](le::event::CursorPos const& pos) { m_input_dispatch.on_cursor_move(pos.normalized.to_target(m_context.framebuffer_size())); },
			[this](le::event::Codepoint const codepoint) { m_input_dispatch.on_codepoint(codepoint); },
			[this](le::event::Key const& key) { m_input_dispatch.on_key(key); },
			[this](le::event::MouseButton const& button) { m_input_dispatch.on_mouse_button(button); },
			[this](le::event::Scroll const& scroll) { m_input_dispatch.on_scroll(scroll); },
		};
		for (auto const& event : events) { std::visit(visitor, event); }
	}
}
} // namespace hog
