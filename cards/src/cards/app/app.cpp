#include "cards/app/app.hpp"
#include "cards/scene/create_terminal.hpp"
#include "cards/scene/lab.hpp"
#include "cards/scene/load_catalog.hpp"
#include "le2d/util.hpp"
#include <imgui.h>

namespace cards {
namespace {
constexpr auto context_ci = le::Context::CreateInfo{
	.window = le::WindowInfo{.size = {1024, 1024}, .title = "cards"},
};
} // namespace

void App::run() {
	m_context = le::Context::create(context_ci);

	auto const assets_dir = le::FileDataLoader::upfind("assets", le::util::exe_path());
	m_data_loader.set_root_dir(assets_dir);
	m_waiter = m_context->create_waiter();

	m_asset_map.emplace(m_context.get());

	m_scene_factory.emplace(this);
	m_scene_factory->store_creator("Lab", [] { return std::make_unique<scene::Lab>(); });
	m_scene_factory->store_creator("LoadCatalog", [this] { return std::make_unique<scene::LoadCatalog>(&m_catalog, &*m_asset_map, "CreateTerminal"); });
	m_scene_factory->store_creator("CreateTerminal", [this] { return std::make_unique<scene::CreateTerminal>(&m_console, &m_catalog, "Lab"); });

	m_scene_factory->set_next_scene("LoadCatalog");

	while (m_context->is_running()) {
		m_context->next_frame();
		m_unprojector = m_context->unprojector(m_context->get_renderer().get_viewport(), {});

		if (auto next_scene = m_scene_factory->get_next_scene()) {
			m_context->wait_idle();
			m_scene = std::move(next_scene);
		}

		m_input_router->dispatch(m_context->event_queue());

		auto const dt = m_context->get_frame_stats().total_dt;
		if (m_console.terminal) { m_console.terminal->tick(dt); }

		m_scene->tick_frame(dt);
		auto& renderer = m_context->begin_render(m_scene->clear_color);
		m_scene->render_frame(renderer);
		if (m_console.terminal) { m_console.terminal->draw(renderer); }

		m_context->present();
	}
}
} // namespace cards
