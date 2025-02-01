#include <klib/task/queue.hpp>
#include <le2d/asset/store.hpp>
#include <le2d/context.hpp>
#include <log.hpp>
#include <scene/lab.hpp>

namespace hog::scene {
Lab::Lab(gsl::not_null<le::ServiceLocator*> services) : Scene(services) {
	load_fonts();
	create_textures();
	m_quad.texture = &m_textures.front();
}

void Lab::on_event(le::event::Key const key) {
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
}

void Lab::tick(kvf::Seconds const dt) {
	auto dxy = glm::vec2{};
	if (m_held_keys.left) { dxy.x += -1.0f; }
	if (m_held_keys.right) { dxy.x += 1.0f; }
	if (std::abs(dxy.x) > 0.0f) { dxy = glm::normalize(dxy); }
	m_render_view.position.x += 100.0f * dxy.x * dt.count();
}

void Lab::render(le::Renderer& renderer) const {
	auto n_viewport = kvf::uv_rect_v;
	auto const polygon_mode = vk::PolygonMode::eFill;
	renderer.set_line_width(3.0f);
	renderer.polygon_mode = polygon_mode;

	// n_viewport.rb.x = 0.5f;
	renderer.set_render_area(n_viewport);
	renderer.view = m_render_view;
	m_quad.draw(renderer);

	m_text.draw(renderer);

	n_viewport = {.lt = {0.5f, 0.25f}, .rb = {0.75f, 0.5f}};
	renderer.set_render_area(n_viewport);
	renderer.view = {};
	renderer.view.scale = glm::vec2{0.25f};
	m_quad.draw(renderer);
}

void Lab::reset_events() { m_held_keys = {}; }

void Lab::load_fonts() {
	auto queue = klib::task::Queue{};

	auto& context = m_services->get<le::Context>();
	auto load_task = context.create_asset_load_task(&queue);
	load_task->enqueue<le::Font>("font.ttf");

	queue.enqueue(*load_task);

	auto& asset_store = m_services->get<le::asset::Store>();
	auto const loaded = load_task->transfer_loaded(asset_store);
	log::debug("{} assets loaded", loaded);

	if (auto* main_font = asset_store.get<le::Font>("font.ttf")) { m_text.set_string(*main_font, "multi-line text\ndemo. it works!"); }
}

void Lab::create_textures() {
	auto& context = m_services->get<le::Context>();

	auto pixels = kvf::ColorBitmap{glm::ivec2{2, 2}};
	pixels[0, 0] = kvf::red_v;
	pixels[1, 0] = kvf::green_v;
	pixels[0, 1] = kvf::blue_v;
	pixels[1, 1] = kvf::white_v;
	auto texture = le::Texture{context.create_texture(pixels.bitmap())};
	texture.sampler.min_filter = texture.sampler.mag_filter = vk::Filter::eNearest;
	m_textures.push_back(std::move(texture));

	pixels = kvf::ColorBitmap{glm::ivec2{2, 1}};
	pixels[0, 0] = kvf::cyan_v;
	pixels[1, 0] = kvf::yellow_v;
	texture = le::Texture{context.create_texture(pixels.bitmap())};
	texture.sampler = m_textures.back().sampler;
	m_textures.push_back(std::move(texture));
}
} // namespace hog::scene
