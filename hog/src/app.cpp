#include <app.hpp>
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

struct App::EventVisitor {
	App& self;

	void operator()(le::event::WindowClose const close) const { self.m_scene->on_event(close); }

	void operator()(le::event::WindowFocus const focus) const { self.m_scene->on_event(focus); }

	void operator()(le::event::CursorFocus const focus) const { self.m_scene->on_event(focus); }

	void operator()(le::event::FramebufferResize const resize) const {
		self.m_scene->on_event(resize);
		if (self.m_terminal) { self.m_terminal->resize(resize); }
	}

	void operator()(le::event::WindowResize const resize) const { self.m_scene->on_event(resize); }

	void operator()(le::event::WindowPos const pos) const { self.m_scene->on_event(pos); }

	void operator()(le::event::CursorPos const& cursor_pos) const {
		if (!self.m_terminal || !self.m_terminal->is_active()) { self.m_scene->on_event(cursor_pos); }

		if (self.m_terminal) { self.m_terminal->on_cursor_move(cursor_pos); }
	}

	void operator()(le::event::Codepoint const codepoint) const {
		if (!self.m_terminal || !self.m_terminal->is_active()) { self.m_scene->on_event(codepoint); }

		if (self.m_terminal) { self.m_terminal->on_codepoint(codepoint); }
	}

	void operator()(le::event::Key const& key) const {
		if (!self.m_terminal || !self.m_terminal->is_active()) { self.m_scene->on_event(key); }

		if (key.action == GLFW_RELEASE && key.key == GLFW_KEY_W && key.mods == GLFW_MOD_CONTROL) { self.m_context.shutdown(); }

		if (self.m_terminal) {
			self.m_terminal->on_key(key);
			if (!self.m_was_terminal_active && self.m_terminal->is_active()) { self.m_scene->disengage_input(); }
			self.m_was_terminal_active = self.m_terminal->is_active();
		}
	}

	void operator()(le::event::MouseButton const& button) const { self.m_scene->on_event(button); }

	void operator()(le::event::Scroll const scroll) const {
		if (!self.m_terminal || !self.m_terminal->is_active()) { self.m_scene->on_event(scroll); }

		if (self.m_terminal) { self.m_terminal->on_scroll(scroll); }
	}

	void operator()(le::event::Drop const drop) const { self.m_scene->on_event(drop); }
};

App::App(gsl::not_null<le::IDataLoader const*> data_loader) : m_context(data_loader, context_ci) {}

void App::run() {
	m_blocker = m_context.create_device_block();

	auto font_loader = le::asset::FontLoader{&m_context};
	if (auto font = font_loader.load("mono.ttf")) {
		m_terminal.emplace(&font->asset, m_context.framebuffer_size());
		m_terminal->add_command("quit", {}, [&](le::console::Printer& printer) {
			printer.println("quitting");
			m_context.shutdown();
		});
		m_asset_store.insert_base("mono.ttf", std::move(font));
	}

	m_services.bind(&m_context);
	m_services.bind(&m_asset_store);
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

	if (m_terminal) { m_terminal->tick(dt); }
}

void App::render(le::Renderer& renderer) const {
	m_scene->render(renderer);

	if (m_terminal) { m_terminal->draw(renderer); }
}

void App::process_events() {
	auto const visitor = EventVisitor{*this};
	for (auto const& event : m_context.event_queue()) { std::visit(visitor, event); }
}
} // namespace hog
