#include <imgui.h>
#include <app.hpp>
#include <applet/flipbook_editor.hpp>
#include <applet/font_viewer.hpp>
#include <applet/tile_sheet_editor.hpp>
#include <klib/visitor.hpp>
#include <log.hpp>
#include <algorithm>

namespace le::assed {
namespace {
constexpr auto context_create_info_v = ContextCreateInfo{
	.window = WindowInfo{.size = {600, 600}, .title = "Asset Editor"},
	.framebuffer_samples = vk::SampleCountFlagBits::e2,
};

template <std::derived_from<Applet> T>
auto create_applet(gsl::not_null<ServiceLocator const*> services) -> std::unique_ptr<Applet> {
	return std::make_unique<T>(services);
}
} // namespace

App::App(FileDataLoader data_loader) : m_data_loader(std::move(data_loader)), m_context(Context::create(context_create_info_v)) {}

void App::run() {
	m_waiter = m_context->create_waiter();
	m_asset_loader = m_context->create_asset_loader(&m_data_loader);

	m_service_locator.bind(&m_data_loader);
	m_service_locator.bind(m_context.get());
	m_service_locator.bind(&m_input_dispatch);
	m_service_locator.bind(&m_asset_loader);
	create_factories();

	set_applet(m_factories.front());

	while (m_context->is_running()) {
		m_context->next_frame();
		swap_applet();
		handle_events();
		tick();
		render();
		m_context->present();
	}
}

void App::create_factories() {
	m_factories = {
		Factory{.name = TileSheetEditor::name_v, .create = &create_applet<TileSheetEditor>},
		Factory{.name = FlipbookEditor::name_v, .create = &create_applet<FlipbookEditor>},
		Factory{.name = FontViewer::name_v, .create = &create_applet<FontViewer>},
	};
}

void App::swap_applet() {
	if (m_next_applet.empty()) { return; }
	auto const name = m_next_applet;
	m_next_applet = {};

	auto const it = std::ranges::find_if(m_factories, [name](Factory const& f) { return f.name.as_view() == name; });
	if (it == m_factories.end()) {
		log.error("unrecognized applet: {}", name);
		return;
	}

	set_applet(*it);
}

void App::handle_events() {
	auto& dispatch = m_input_dispatch;
	auto const fb_size = m_context->main_pass_size();
	auto const visitor = klib::SubVisitor{
		[this](event::WindowClose) { try_exit(); },
		[&dispatch, fb_size](event::CursorPos const& cursor) { dispatch.on_cursor_move(cursor.normalized.to_target(fb_size)); },
		[&dispatch](event::Codepoint const codepoint) { dispatch.on_codepoint(codepoint); },
		[&dispatch](event::Key const& key) { dispatch.on_key(key); },
		[&dispatch](event::MouseButton const& button) { dispatch.on_mouse_button(button); },
		[&dispatch](event::Scroll const& scroll) { dispatch.on_scroll(scroll); },
		[&dispatch](event::Drop const& drop) { dispatch.on_drop(drop); },
	};
	for (auto const& event : m_context->event_queue()) { std::visit(visitor, event); }
}

void App::tick() {
	auto const dt = m_delta_time.tick();
	m_applet->do_tick(dt);

	main_menu();
}

void App::render() {
	if (auto& renderer = m_context->begin_render(m_applet->clear_color)) {
		m_applet->render(renderer);
		renderer.end_render();
	}
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
		m_applet->populate_file_menu();
		ImGui::Separator();
		if (ImGui::MenuItem("Exit")) { try_exit(); }
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

void App::set_applet(Factory const& factory) {
	m_context->wait_idle();
	m_applet = factory.create(&m_service_locator);
	m_applet->do_setup();
	log.info("loaded '{}'", factory.name.as_view());
}

void App::try_exit() {
	if (!m_applet->try_exit()) {
		m_context->cancel_window_close();
	} else {
		m_context->set_window_close();
	}
}
} // namespace le::assed
