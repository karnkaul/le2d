#pragma once
#include "clap/result.hpp"
#include "demo/scene/scene.hpp"
#include "le2d/context.hpp"
#include "le2d/file_data_loader.hpp"
#include <functional>

namespace demo {
class App {
  public:
	auto run(int argc, char const* const* argv) -> int;

  private:
	struct SceneInfo {
		std::string_view name{};
		std::move_only_function<std::unique_ptr<Scene>()> factory{};
	};

	struct Params {
		bool force_x11{};
	};

	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> clap::Result;

	void create_context();
	void create_data_loader();
	void add_scene_infos();

	template <std::derived_from<Scene> SceneTypeT>
	[[nodiscard]] auto create_scene_info() const -> SceneInfo {
		return SceneInfo{
			.name = SceneTypeT::name_v,
			.factory = [this] { return std::make_unique<SceneTypeT>(m_context.get(), &*m_data_loader); },
		};
	}

	void set_active_scene(std::unique_ptr<Scene> scene);

	void run_loop();

	void inspect_main_menu();

	Params m_params{};

	std::unique_ptr<le::Context> m_context{};
	std::vector<SceneInfo> m_scene_infos{};
	std::unique_ptr<Scene> m_active_scene{};
	std::optional<le::FileDataLoader> m_data_loader{};

	le::Context::Waiter m_waiter{};
};
} // namespace demo
