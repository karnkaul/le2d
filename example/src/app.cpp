#include <app.hpp>
#include <log.hpp>

namespace le::example {
namespace {
auto const context_ci = ContextCreateInfo{
	.window = WindowInfo{.size = {1280, 720}, .title = "le2d example"},
	// .window = FullscreenInfo{.title = "le2d example"},
	.default_shader = {.vertex = "default.vert", .fragment = "default.frag"},
	.framebuffer_samples = vk::SampleCountFlagBits::e2,
};
}

App::App(gsl::not_null<IDataLoader const*> data_loader)
	: m_context(data_loader, context_ci), m_main_font(m_context.create_font()), m_mono_font(m_context.create_font()) {}

void App::run() {
	m_blocker = m_context.create_device_block();

	create_textures();
	load_fonts();

	m_quad.texture = &m_textures.front();

	if (m_mono_font.is_loaded()) {
		auto const cci = console::TerminalCreateInfo{
			.style = {.text_height = TextHeight{20}},
		};
		m_terminal.emplace(&m_mono_font, m_context.get_render_window().framebuffer_size(), cci);

		m_terminal->add_command("quit", "shutdown app", [this](console::Printer& printer) {
			printer.println("quitting");
			m_context.shutdown();
		});
	}

	m_delta_time.reset();

	while (m_context.is_running()) {
		m_context.next_frame();

		auto const dt = m_delta_time.tick();
		tick(dt);
		if (auto renderer = m_context.begin_render()) { render(renderer); }

		m_context.present();
	}
}

void App::load_fonts() {
	load_font(m_main_font, "font.ttf");
	load_font(m_mono_font, "mono.ttf");

	if (m_main_font.is_loaded()) { m_text.set_string(m_main_font, "multi-line text\ndemo. it works!"); }
}

void App::load_font(Font& out, Uri const& uri) {
	auto bytes = std::vector<std::byte>{};
	if (!m_context.get_data_loader().load_bytes(bytes, uri)) {
		log::error("Failed to load font bytes: '{}'", uri.get_string());
		return;
	}

	out.load_face(std::move(bytes));
	if (!out.is_loaded()) {
		log::error("Failed to load font: '{}'", uri.get_string());
		return;
	}

	log::debug("Font '{}' loaded from '{}'", out.get_name().as_view(), uri.get_string());
}

void App::create_textures() {
	auto pixels = kvf::ColorBitmap{glm::ivec2{2, 2}};
	pixels[0, 0] = kvf::red_v;
	pixels[1, 0] = kvf::green_v;
	pixels[0, 1] = kvf::blue_v;
	pixels[1, 1] = kvf::white_v;
	auto texture = Texture{m_context.create_texture(pixels.bitmap())};
	texture.sampler.min_filter = texture.sampler.mag_filter = vk::Filter::eNearest;
	m_textures.push_back(std::move(texture));

	pixels = kvf::ColorBitmap{glm::ivec2{2, 1}};
	pixels[0, 0] = kvf::cyan_v;
	pixels[1, 0] = kvf::yellow_v;
	texture = Texture{m_context.create_texture(pixels.bitmap())};
	texture.sampler = m_textures.back().sampler;
	m_textures.push_back(std::move(texture));
}

void App::tick(kvf::Seconds const dt) {
	process_events();

	if (!m_terminal || !m_terminal->is_active()) {
		auto dxy = glm::vec2{};
		if (m_held_keys.left) { dxy.x += -1.0f; }
		if (m_held_keys.right) { dxy.x += 1.0f; }
		if (std::abs(dxy.x) > 0.0f) { dxy = glm::normalize(dxy); }
		m_render_view.position.x += 100.0f * dxy.x * dt.count();
		// m_render_view.orientation += 10.0f * dt.count();
	}

	if (m_terminal) { m_terminal->tick(dt); }
}

void App::render(Renderer& renderer) const {
	auto n_viewport = kvf::uv_rect_v;
	auto const polygon_mode = vk::PolygonMode::eFill;
	renderer.set_line_width(3.0f);
	renderer.polygon_mode = polygon_mode;

	// n_viewport.rb.x = 0.5f;
	renderer.set_render_area(n_viewport);
	renderer.view = m_render_view;
	m_quad.draw(renderer);

	// if (m_input_text) { m_input_text->draw(renderer); }
	m_text.draw(renderer);

	n_viewport = {.lt = {0.5f, 0.25f}, .rb = {0.75f, 0.5f}};
	renderer.set_render_area(n_viewport);
	renderer.view = {};
	renderer.view.scale = glm::vec2{0.25f};
	m_quad.draw(renderer);

	if (m_terminal) { m_terminal->draw(renderer); }
}

void App::process_events() {
	auto const visitor = klib::SubVisitor{
		[&](event::Key const& key) {
			if (key.key == GLFW_KEY_LEFT) {
				switch (key.action) {
				case GLFW_PRESS: m_held_keys.left = true; break;
				case GLFW_RELEASE: m_held_keys.left = false; break;
				}
			}
			if (key.key == GLFW_KEY_RIGHT) {
				switch (key.action) {
				case GLFW_PRESS: m_held_keys.right = true; break;
				case GLFW_RELEASE: m_held_keys.right = false; break;
				}
			}

			if (key.action == GLFW_RELEASE && key.key == GLFW_KEY_W && key.mods == GLFW_MOD_CONTROL) { m_context.shutdown(); }

			if (m_terminal) { m_terminal->on_key(key); }
		},

		[&](event::Codepoint const codepoint) {
			if (m_terminal) { m_terminal->on_codepoint(codepoint); }
		},

		[&](event::FramebufferResize const resize) {
			if (m_terminal) { m_terminal->resize(resize); }
		},

		[&](event::CursorPos const& cursor_pos) {
			if (m_terminal) { m_terminal->on_cursor_move(cursor_pos); }
		},

		[&](event::Scroll const scroll) {
			if (m_terminal) { m_terminal->on_scroll(scroll); }
		},
	};

	for (auto const& event : m_context.event_queue()) { std::visit(visitor, event); }
}
} // namespace le::example
