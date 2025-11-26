#include "le2d/file_data_loader.hpp"
#include "le2d/util.hpp"
#include <GLFW/glfw3.h>
#include <app.hpp>
#include <klib/args/parse.hpp>
#include <klib/log.hpp>
#include <log.hpp>

namespace le::assed {
namespace {
void run(std::string assets_dir) {
	if (assets_dir.empty()) { assets_dir = FileDataLoader::upfind("assets", util::exe_path()); }
	auto app = App{FileDataLoader{assets_dir}};
	app.run();
}
} // namespace
} // namespace le::assed

auto main(int argc, char** argv) -> int {
	try {
		auto assets_dir = std::string{};
		auto force_x11 = false;
		auto const args = std::array{
			klib::args::named_flag(force_x11, "x,force-x11", "Force X11 backend"),
			klib::args::named_option(assets_dir, "a,assets", "path to assets directory"),
		};
		auto const parse_result = klib::args::parse_main(args, argc, argv);
		if (parse_result.early_return()) { return parse_result.get_return_code(); }
		if (force_x11) { glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); }
		le::assed::run(assets_dir);
	} catch (std::exception const& e) {
		le::assed::log.error("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		le::assed::log.error("PANIC!");
		return EXIT_FAILURE;
	}
}
