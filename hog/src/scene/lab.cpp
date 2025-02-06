#include <djson/json.hpp>
#include <klib/task/queue.hpp>
#include <le2d/asset/store.hpp>
#include <le2d/context.hpp>
#include <le2d/vertex_bounds.hpp>
#include <log.hpp>
#include <scene/lab.hpp>
#include <scene/switcher.hpp>

namespace hog::scene {
Lab::Lab(gsl::not_null<le::ServiceLocator*> services) : Scene(services) {
	load_assets();
	create_textures();
	m_quad.texture = &m_textures.front();
	m_line_rect.create(m_quad.get_rect(), kvf::yellow_v);

	m_horz = le::input::KeyAxis{GLFW_KEY_A, GLFW_KEY_D};
	m_rotate = le::input::KeyAxis{GLFW_KEY_E, GLFW_KEY_Q};
	m_escape = le::input::KeyChord{GLFW_KEY_ESCAPE, GLFW_RELEASE};
	m_mb1 = le::input::MouseButtonTrigger{GLFW_MOUSE_BUTTON_1};

	auto const& asset_store = m_services->get<le::asset::Store>();
	if (auto const* texture = asset_store.get<le::Texture>("hog_bg.jpg")) {
		m_background.create(texture->get_size());
		m_background.texture = texture;
	}

	if (auto const* animation = asset_store.get<le::Animation>("animations/lerp.json")) { m_anim.set_animation(animation); }
}

void Lab::on_event(le::event::Key const key) {
	m_horz.on_event(key);
	m_rotate.on_event(key);

	if (m_escape.is_engaged(key)) { m_services->get<ISwitcher>().switch_scene<Scene>(); }
}

void Lab::on_event(le::event::MouseButton const button) {
	if (m_mb1.on_event(button)) { m_prev_cursor_pos = m_cursor_pos; }
}

void Lab::on_event(le::event::CursorPos const pos) { m_cursor_pos = pos.normalized; }

void Lab::on_event(le::event::Scroll const scroll) {
	auto const dscale = m_zoom_speed * scroll.y;
	m_render_view.scale.x = std::clamp(m_render_view.scale.x + dscale, m_level_info.background.min_scale, m_level_info.background.max_scale);
	m_render_view.scale.y = m_render_view.scale.x;
}

void Lab::tick(kvf::Seconds const dt) {
	auto dxy = glm::vec2{};
	dxy.x = m_horz.value();
	if (std::abs(dxy.x) > 0.0f) { dxy = glm::normalize(dxy); }
	m_render_view.position.x += m_translate_speed * dxy.x * dt.count();

	auto const unprojector = get_unprojector(m_render_view);
	auto const cursor_pos = unprojector.unproject(m_cursor_pos);
	if (m_mb1.is_engaged()) {
		auto const prev_fb_cursor = unprojector.unproject(m_prev_cursor_pos);
		auto const cursor_dxy = cursor_pos - prev_fb_cursor;
		m_render_view.position -= cursor_dxy;
	}

	auto const drot = m_rotate.value();
	m_render_view.orientation += 50.0f * drot * dt.count();

	m_anim.tick(dt);
	m_quad.instance.transform = m_anim.get_payload();

	auto const rect = m_quad.bounding_rect();
	if (rect.contains(cursor_pos)) {
		m_quad.instance.tint = kvf::red_v;
	} else {
		m_quad.instance.tint = kvf::white_v;
	}

	inspect();

	m_prev_cursor_pos = m_cursor_pos;
}

void Lab::render(le::Renderer& renderer) const {
	auto n_viewport = kvf::uv_rect_v;
	auto const polygon_mode = vk::PolygonMode::eFill;
	renderer.set_line_width(3.0f);
	renderer.polygon_mode = polygon_mode;

	// n_viewport.rb.x = 0.5f;
	renderer.set_render_area(n_viewport);
	renderer.view = m_render_view;
	m_background.draw(renderer);
	m_quad.draw(renderer);
	m_line_rect.draw(renderer);

	n_viewport = {.lt = {0.5f, 0.25f}, .rb = {0.75f, 0.5f}};
	renderer.set_render_area(n_viewport);
	renderer.view = {};
	renderer.view.scale = glm::vec2{0.25f};
	// m_quad.draw(renderer);
}

void Lab::disengage_input() {
	m_horz.disengage();
	m_rotate.disengage();
	m_mb1.disengage();
}

void Lab::load_assets() {
	auto& context = m_services->get<le::Context>();
	auto const& data_loader = context.get_data_loader();
	auto level_json = dj::Json{};
	if (data_loader.load_json(level_json, "levels/lab.json")) {
		m_level_info = {};
		from_json(level_json, m_level_info);
		log::info("'{}' level loaded", m_level_info.name);
		m_render_view.scale = glm::vec2{m_level_info.background.default_scale};
	}

	auto queue = klib::task::Queue{};

	auto load_task = context.create_asset_load_task(&queue);
	load_task->enqueue<le::Font>("font.ttf");
	load_task->enqueue<le::Animation>("animations/lerp.json");
	for (auto const& texture : m_level_info.assets.textures) { load_task->enqueue<le::Texture>(texture); }

	queue.enqueue(*load_task);

	auto& asset_store = m_services->get<le::asset::Store>();
	auto const loaded = load_task->transfer_loaded(asset_store);
	log::debug("{} assets loaded", loaded);
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

void Lab::inspect() {
	if (ImGui::Begin("Inspect")) {
		ImGui::DragFloat("move speed", &m_translate_speed, 1.0f, 1.0f, 2000.0f);
		ImGui::DragFloat("zoom speed", &m_zoom_speed, 0.01f, 0.01f, 0.5f);
	}
	ImGui::End();
}
} // namespace hog::scene
