#include <le2d/console/tweak.hpp>

namespace le::console {
void ITweak::run(IPrinter& printer, std::string_view const input) {
	if (input.empty()) {
		auto text = std::string{};
		write_to(text);
		printer.println(text);
		return;
	}

	if (!deserialize(input)) {
		printer.printerr(std::format("failed to set value to: '{}'", input));
		return;
	}
}
} // namespace le::console
