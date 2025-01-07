#include <klib/version_str.hpp>
#include <kvf/color_bitmap.hpp>
#include <kvf/time.hpp>
#include <le2d/build_version.hpp>
#include <le2d/context.hpp>
#include <le2d/drawable.hpp>
#include <le2d/file_data_loader.hpp>
#include <log.hpp>

namespace le::example {
namespace {
struct App {
	explicit App(gsl::not_null<IDataLoader const*> data_loader) : m_context(data_loader, context_ci) {}

	void run() {
		m_blocker = m_context.create_device_block();
		create_texture();

		m_delta_time.reset();

		while (m_context.is_running()) {
			m_context.next_frame();

			auto const dt = m_delta_time.tick();
			tick(dt);
			if (auto renderer = m_context.begin_render()) { render(renderer); }

			m_context.present();
		}
	}

  private:
	struct HeldKeys {
		bool left{};
		bool right{};
	};

	inline static auto const context_ci = ContextCreateInfo{
		.window_size = {1280, 720},
		.window_title = "le2d example",
		.default_shader = {.vertex = "default.vert", .fragment = "default.frag"},
		.framebuffer_samples = vk::SampleCountFlagBits::e2,
	};

	void create_texture() {
		auto pixels = kvf::ColorBitmap{glm::ivec2{2, 2}};
		pixels[0, 0] = kvf::red_v;
		pixels[0, 1] = kvf::green_v;
		pixels[1, 0] = kvf::blue_v;
		pixels[1, 1] = kvf::white_v;
		m_texture.emplace(m_context.create_texture(pixels.bitmap()));
		m_texture->sampler.min_filter = m_texture->sampler.mag_filter = vk::Filter::eNearest;
		m_quad.texture = &*m_texture;
	}

	void tick(kvf::Seconds const dt) {
		process_events();

		auto dxy = glm::vec2{};
		if (m_held_keys.left) { dxy.x += -1.0f; }
		if (m_held_keys.right) { dxy.x += 1.0f; }
		if (std::abs(dxy.x) > 0.0f) { dxy = glm::normalize(dxy); }
		m_render_view.position.x += 100.0f * dxy.x * dt.count();
		m_render_view.orientation += 10.0f * dt.count();
	}

	void render(Renderer& renderer) const {
		auto n_viewport = kvf::uv_rect_v;
		auto const polygon_mode = vk::PolygonMode::eFill;
		renderer.set_line_width(3.0f);
		renderer.polygon_mode = polygon_mode;

		// n_viewport.rb.x = 0.5f;
		renderer.set_render_area(n_viewport);
		renderer.view = m_render_view;

		m_quad.draw(renderer);

		n_viewport = {.lt = {0.5f, 0.25f}, .rb = {0.75f, 0.5f}};
		renderer.set_render_area(n_viewport);
		renderer.view = {};
		renderer.view.scale = glm::vec2{0.25f};
		m_quad.draw(renderer);
	}

	void process_events() {
		auto const on_release = [this](int const key) {
			switch (key) {
			case GLFW_KEY_ESCAPE: m_context.shutdown();
			default: break;
			}
		};

		auto const visitor = SubVisitor{
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
				if (key.action == GLFW_RELEASE && key.mods == 0) { on_release(key.key); }
			},
		};

		for (auto const& event : m_context.event_queue()) { std::visit(visitor, event); }
	}

	Context m_context;

	std::optional<Texture> m_texture{};
	Quad m_quad{};

	kvf::DeltaTime m_delta_time{};
	HeldKeys m_held_keys{};
	Transform m_render_view{};

	kvf::DeviceBlock m_blocker;
};
} // namespace
} // namespace le::example

auto main(int argc, char** argv) -> int {
	auto const file = klib::log::File{"le2d-debug.log"};
	try {
		auto const arg0 = [&] -> std::string_view {
			if (argc < 1) { return {}; }
			return *argv;
		}();
		auto const assets_dir = le::FileDataLoader::upfind("assets", arg0);
		auto const data_loader = le::FileDataLoader{assets_dir};

		le::example::log::info("le2d {}", klib::to_string(le::build_version_v));
		le::example::App{&data_loader}.run();
	} catch (std::exception const& e) {
		le::example::log::error("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		le::example::log::error("PANIC!");
		return EXIT_FAILURE;
	}
}
