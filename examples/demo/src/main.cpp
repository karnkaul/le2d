#include "demo/app.hpp"
#include <print>

int main(int argc, char** argv) {
	try {
		return demo::App{}.run(argc, argv);
	} catch (std::exception const& e) {
		std::println("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}
}
