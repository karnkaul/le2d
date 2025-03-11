#include <app.hpp>
#include <klib/log.hpp>
#include <le2d/file_data_loader.hpp>
#include <le2d/util.hpp>
#include <log.hpp>

namespace le::assed {
namespace {
void run(ShaderUris const& shader_uris = {}) {
	auto const exe_path = util::exe_path();
	auto assets_dir = FileDataLoader::upfind("assets", exe_path);
	if (assets_dir.empty()) { assets_dir = "."; }
	auto data_loader = FileDataLoader{assets_dir};

	auto app = App{&data_loader, shader_uris};
	app.run();
}
} // namespace
} // namespace le::assed

auto main() -> int {
	try {
		le::assed::run();
	} catch (std::exception const& e) {
		le::assed::log::error("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		le::assed::log::error("PANIC!");
		return EXIT_FAILURE;
	}
}
