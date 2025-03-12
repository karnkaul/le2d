#include <imgui.h>
#include <app.hpp>
#include <klib/visitor.hpp>
#include <log.hpp>
#include <algorithm>

namespace le::assed {
namespace {
constexpr auto context_create_info_v(ShaderUris const& shader_uris) {
	return ContextCreateInfo{
		.window = WindowInfo{.size = {600, 600}, .title = "Asset Editor", .decorated = true},
		.default_shader_uri = {.vertex = shader_uris.vertex, .fragment = shader_uris.fragment},
		.framebuffer_samples = vk::SampleCountFlagBits::e2,
	};
}

template <std::derived_from<Applet> T>
auto create_applet(gsl::not_null<ServiceLocator const*> services) -> std::unique_ptr<Applet> {
	return std::make_unique<T>(services);
}
} // namespace

App::App(FileDataLoader data_loader, ShaderUris const& shader_uris)
	: m_data_loader(std::move(data_loader)), m_context(&m_data_loader, context_create_info_v(shader_uris)) {}

void App::run() {
	m_blocker = m_context.get_render_window().get_render_device().get_device();

	m_service_locator.bind(&m_data_loader);
	m_service_locator.bind(&m_context);
	m_service_locator.bind(&m_input_dispatch);
	create_factories();

	m_applet = m_factories.front().create(&m_service_locator);

	while (m_context.is_running()) {
		m_context.next_frame();
		swap_applet();
		handle_events();
		tick();
		render();
		m_context.present();
	}
}

void App::create_factories() {
	m_factories = {
		Factory{.name = "Applet", .create = &create_applet<Applet>},
	};
}

void App::swap_applet() {
	if (m_next_applet.empty()) { return; }
	auto const name = m_next_applet;
	m_next_applet = {};

	auto const it = std::ranges::find_if(m_factories, [name](Factory const& f) { return f.name.as_view() == name; });
	if (it == m_factories.end()) {
		log::error("unrecognized applet: {}", name);
		return;
	}

	m_blocker.get().waitIdle();
	m_applet = it->create(&m_service_locator);
	log::info("loaded '{}'", name);
}

void App::handle_events() {
	auto& dispatch = m_input_dispatch;
	auto const fb_size = m_context.framebuffer_size();
	auto const visitor = klib::SubVisitor{
		[&dispatch, fb_size](event::CursorPos const& cursor) { dispatch.on_cursor_move(cursor.normalized.to_target(fb_size)); },
		[&dispatch](event::Codepoint const codepoint) { dispatch.on_codepoint(codepoint); },
		[&dispatch](event::Key const& key) { dispatch.on_key(key); },
		[&dispatch](event::MouseButton const& button) { dispatch.on_mouse_button(button); },
		[&dispatch](event::Scroll const& scroll) { dispatch.on_scroll(scroll); },
		[&dispatch](event::Drop const& drop) { dispatch.on_drop(drop); },
	};
	for (auto const& event : m_context.event_queue()) { std::visit(visitor, event); }
}

void App::tick() {
	auto const dt = m_delta_time.tick();
	m_applet->tick(dt);

	main_menu();
}

void App::render() {
	if (auto renderer = m_context.begin_render()) { m_applet->render(renderer); }
}

void App::main_menu() {
	if (ImGui::BeginMainMenuBar()) {
		file_menu();
		applet_menu();
		m_applet->populate_menu_bar();
		ImGui::EndMainMenuBar();
	}
}

void App::file_menu() {
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Save", nullptr, false, m_applet->has_unsaved_changes())) { m_applet->on_save(); }
		ImGui::Separator();
		if (ImGui::MenuItem("Exit")) { m_context.shutdown(); }
		ImGui::EndMenu();
	}
}

void App::applet_menu() {
	if (ImGui::BeginMenu("Applets")) {
		for (auto const& factory : m_factories) {
			if (ImGui::MenuItem(factory.name.c_str())) { m_next_applet = factory.name.as_view(); }
		}
		ImGui::EndMenu();
	}
}
} // namespace le::assed
