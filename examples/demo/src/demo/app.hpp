#pragma once
#include "clap/result.hpp"
#include "demo/scene/scene.hpp"
#include "le2d/context.hpp"
#include "le2d/file_data_loader.hpp"

namespace demo {
class App {
  public:
	auto run(int argc, char const* const* argv) -> int;

  private:
	struct Params {
		bool force_x11{};
	};

	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> clap::Result;

	void create_context();
	void create_data_loader();
	void run_loop();

	Params m_params{};

	std::unique_ptr<le::Context> m_context{};
	std::unique_ptr<Scene> m_active_scene{};
	std::optional<le::FileDataLoader> m_data_loader{};
};
} // namespace demo
