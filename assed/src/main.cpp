#include <app.hpp>
#include <klib/args/parse.hpp>
#include <klib/log.hpp>
#include <le2d/file_data_loader.hpp>
#include <le2d/util.hpp>
#include <log.hpp>

namespace le::assed {
namespace {
void run(std::string spirv_dir) {
	if (spirv_dir.empty()) { spirv_dir = FileDataLoader::upfind("spir_v", util::exe_path()); }
	auto const data_loader = FileDataLoader{spirv_dir};
	auto app = App{&data_loader};
	app.run();
}
} // namespace
} // namespace le::assed

auto main(int argc, char** argv) -> int {
	try {
		auto spirv_dir = std::string{};
		auto const args = std::array{
			klib::args::named_option(spirv_dir, "s,spirv-dir", "(relative) path to directory containing SPIR-V shaders"),
		};
		auto const parse_result = klib::args::parse_main(args, argc, argv);
		if (parse_result.early_return()) { return parse_result.get_return_code(); }
		le::assed::run(spirv_dir);
	} catch (std::exception const& e) {
		le::assed::log::error("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		le::assed::log::error("PANIC!");
		return EXIT_FAILURE;
	}
}
