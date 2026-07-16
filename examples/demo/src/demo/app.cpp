#include "demo/app.hpp"
#include "clap/parser.hpp"
#include "demo/scene/console.hpp"
#include "demo/scene/input_actions.hpp"
#include "demo/scene/load_assets.hpp"
#include "le2d/build_version.hpp"
#include "le2d/util.hpp"
#include <imgui.h>

namespace demo {
auto App::run(int const argc, char const* const* const argv) -> int {
	auto const parse_result = parse_args(argc, argv);
	if (parse_result.should_early_exit()) { return parse_result.return_code(); }

	create_context();
	create_data_loader();
	add_scene_infos();
	set_active_scene(m_scene_infos.front().factory());

	run_loop();

	return EXIT_SUCCESS;
}

auto App::parse_args(int argc, char const* const* argv) -> clap::Result {
	auto const build_version = std::format("{}", le::build_version_v);
	auto spec = clap::spec::Parameters{
		.parameters =
			{
				clap::named_flag(m_params.force_x11, "x,force-x11", "Force X11 (only relevant on Linux)"),
			},
		.program =
			{
				.name = "le-demo",
				.version = build_version,
			},
	};
	auto parser = clap::Parser{std::move(spec)};
	return parser.parse_main(argc, argv);
}

void App::create_context() {
	auto const window_info = le::WindowInfo{
		.size = {1280, 720},
		.title = "le-demo",
	};

	auto context_ci = le::Context::CreateInfo{
		.window = window_info,
	};
	if (m_params.force_x11) { context_ci.platform_flags |= le::PlatformFlag::ForceX11; }

	m_context = le::Context::create(context_ci);
}

void App::create_data_loader() {
	auto assets_directory = le::FileDataLoader::upfind("assets", le::util::exe_path());
	m_data_loader.emplace(std::move(assets_directory));
}

void App::add_scene_infos() {
	m_scene_infos.push_back(create_scene_info<scene::LoadAssets>());
	m_scene_infos.push_back(create_scene_info<scene::InputActions>());
	m_scene_infos.push_back(create_scene_info<scene::Console>());
}

void App::set_active_scene(std::unique_ptr<Scene> scene) {
	m_active_scene = std::move(scene);
	m_context->set_title(std::format("le-demo - {} [{}]", m_active_scene->get_name(), le::build_version_v));
}

void App::run_loop() {
	auto delta_time = kvf::DeltaTime{};
	while (m_context->is_running()) {
		m_context->next_frame();
		auto const dt = delta_time.tick();

		m_active_scene->tick_frame(dt);
		m_active_scene->render_frame();

		inspect_main_menu();

		m_context->present();
	}
}

void App::inspect_main_menu() {
	if (!ImGui::BeginMainMenuBar()) { return; }

	auto selected = klib::Ptr<SceneInfo>{};
	if (ImGui::BeginMenu("Scenes")) {
		for (auto& info : m_scene_infos) {
			if (ImGui::MenuItem(info.name.data())) { selected = &info; }
		}
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	if (selected) {
		m_context->wait_idle();
		set_active_scene(selected->factory());
	}
}
} // namespace demo
