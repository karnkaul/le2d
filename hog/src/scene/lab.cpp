#include <djson/json.hpp>
#include <klib/fixed_string.hpp>
#include <klib/task/queue.hpp>
#include <le2d/asset/store.hpp>
#include <le2d/context.hpp>
#include <le2d/event/dispatch.hpp>
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

	m_escape = le::input::KeyChord{GLFW_KEY_ESCAPE, GLFW_RELEASE};
	m_mb1 = le::input::MouseButtonTrigger{GLFW_MOUSE_BUTTON_1};

	auto const& asset_store = m_services->get<le::asset::Store>();
	auto const& level_assets = m_level_info.assets;
	if (auto const* texture = asset_store.get<le::Texture>(level_assets.textures.at(m_level_info.background.texture))) {
		m_background.create(texture->get_size());
		m_background.texture = texture;
	}

	m_props.reserve(m_level_info.props.size());
	for (auto const& prop_info : m_level_info.props) { m_props.push_back(create_prop(asset_store, level_assets, prop_info)); }

	m_button.set_framebuffer_size(m_services->get<le::Context>().framebuffer_size());
	m_button.set_size(glm::vec2{200.0f, 100.0f});
	m_button.set_position({200.0f, -100.0f});
	if (auto* font = asset_store.get<le::Font>("font.ttf")) { m_button.set_text(*font, "click"); }
	m_button.set_on_click([] { log::debug("clicked"); });

	services->get<le::event::Dispatch>().attach(&m_button);
}

void Lab::on_event(le::event::Key const key) {
	if (m_escape.is_engaged(key)) { m_services->get<ISwitcher>().switch_scene<Scene>(); }
}

void Lab::on_event(le::event::MouseButton const button) {
	if (m_mb1.on_event(button)) { m_prev_cursor_pos = m_cursor_pos; }
	// m_button.on_button(button);
}

void Lab::on_event(le::event::CursorPos const pos) {
	m_cursor_pos = pos.normalized;
	// m_button.on_cursor(pos);
}

void Lab::on_event(le::event::Scroll const scroll) {
	auto const dscale = m_zoom_speed * scroll.y;
	m_world_view.scale.x = std::clamp(m_world_view.scale.x + dscale, m_level_info.background.min_scale, m_level_info.background.max_scale);
	m_world_view.scale.y = m_world_view.scale.x;
}

void Lab::tick(kvf::Seconds const dt) {
	auto const unprojector = get_unprojector(m_world_view);
	auto const cursor_pos = unprojector.unproject(m_cursor_pos);
	if (m_mb1.is_engaged()) {
		auto const prev_fb_cursor = unprojector.unproject(m_prev_cursor_pos);
		auto const cursor_dxy = cursor_pos - prev_fb_cursor;
		m_world_view.position -= cursor_dxy;
	}

	for (auto& prop : m_props) { prop.tick(dt); }

	auto const rect = m_quad.bounding_rect();
	if (rect.contains(cursor_pos)) {
		m_quad.instance.tint = kvf::red_v;
	} else {
		m_quad.instance.tint = kvf::white_v;
	}

	m_button.set_framebuffer_size(m_services->get<le::Context>().framebuffer_size());
	m_button.tick(dt);

	// if (m_button.get_state() == ui::WidgetState::Press) { m_mb1.disengage(); }

	inspect();

	m_prev_cursor_pos = m_cursor_pos;
}

void Lab::render(le::Renderer& renderer) const {
	renderer.set_line_width(3.0f);
	renderer.view = m_world_view;
	render_world(renderer);

	renderer.view = {};
	render_ui(renderer);
}

void Lab::disengage_input() {
	m_mb1.disengage();
	m_button.disengage();
}

void Lab::load_assets() {
	auto& context = m_services->get<le::Context>();
	auto const& data_loader = context.get_data_loader();
	auto level_json = dj::Json{};
	if (data_loader.load_json(level_json, "levels/lab.json")) {
		m_level_info = {};
		from_json(level_json, m_level_info);
		log::info("'{}' level loaded", m_level_info.name);
		m_world_view.scale = glm::vec2{m_level_info.background.default_scale};
	}

	auto queue = klib::task::Queue{};

	auto load_task = context.create_asset_load_task(&queue);
	load_task->enqueue<le::Font>("font.ttf");
	for (auto const& texture : m_level_info.assets.textures) { load_task->enqueue<le::Texture>(texture); }
	for (auto const& animation : m_level_info.assets.animations) { load_task->enqueue<le::Animation>(animation); }
	for (auto const& flipbook : m_level_info.assets.flipbooks) { load_task->enqueue<le::Flipbook>(flipbook); }

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
		ImGui::DragFloat2("view position", &m_world_view.position.x, 1.0f);
		if (ImGui::DragFloat("view scale", &m_world_view.scale.x, 0.01f, m_level_info.background.min_scale, m_level_info.background.max_scale)) {
			m_world_view.scale.y = m_world_view.scale.x;
		}
		ImGui::DragFloat("zoom speed", &m_zoom_speed, 0.01f, 0.01f, 0.5f);
		auto disabled = m_button.get_state() == ui::WidgetState::Disabled;
		if (ImGui::Checkbox("disabled", &disabled)) { m_button.set_disabled(disabled); }

		auto params = m_button.get_superellipse_params();
		auto modified = false;
		modified |= ImGui::DragFloat2("size", &params.size.x);
		modified |= ImGui::DragFloat("exponent", &params.exponent);
		auto resolution = int(params.resolution);
		modified |= ImGui::DragInt("resolution", &resolution);
		params.resolution = std::int32_t(resolution);
		if (modified) { m_button.set_superellipse_params(params); }
	}
	ImGui::End();
}

void Lab::render_world(le::Renderer& renderer) const {
	m_background.draw(renderer);
	m_quad.draw(renderer);
	m_line_rect.draw(renderer);

	for (auto const& prop : m_props) { prop.draw(renderer); }
}

void Lab::render_ui(le::Renderer& renderer) const {
	m_button.draw(renderer);
	//
}
} // namespace hog::scene
