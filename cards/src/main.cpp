#include <app.hpp>
#include <log.hpp>
#include <exception>

auto main() -> int {
	auto const file = klib::log::File{"cards.log"};
	try {
		cards::App{}.run();
	} catch (std::exception const& e) {
		cards::log.error("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		cards::log.error("PANIC!");
		return EXIT_FAILURE;
	}
}
