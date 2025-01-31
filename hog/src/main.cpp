#include <app.hpp>
#include <klib/args/parse.hpp>
#include <klib/version_str.hpp>
#include <le2d/build_version.hpp>
#include <le2d/file_data_loader.hpp>
#include <log.hpp>

auto main(int argc, char** argv) -> int {
	auto const file = klib::log::File{"le2d-debug.log"};
	try {
		auto force_x11 = false;
		auto assets_dir = std::string{};
		auto const args = std::array{
			klib::args::named_flag(force_x11, "x,force-x11", "force X11 instead of Wayland (Linux)"),
			klib::args::positional_optional(assets_dir, "assets/", "assets directory"),
		};
		auto const parse_result = klib::args::parse_main(args, argc, argv);
		if (parse_result.early_return()) { return parse_result.get_return_code(); }

		if (assets_dir.empty()) { assets_dir = le::FileDataLoader::upfind("assets", *argv); }
		auto const data_loader = le::FileDataLoader{assets_dir};

		hog::log::info("le2d {}", klib::to_string(le::build_version_v));

		if (force_x11) { glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); }
		hog::App{&data_loader}.run();
	} catch (std::exception const& e) {
		hog::log::error("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		hog::log::error("PANIC!");
		return EXIT_FAILURE;
	}
}
