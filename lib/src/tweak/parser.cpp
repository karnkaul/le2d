#include "le2d/tweak/parser.hpp"

namespace le::tweak {
auto Parser<bool>::parse(std::string_view const in, bool& out) -> bool {
	if (in == "true" || in == "1") {
		out = true;
		return true;
	}
	if (in == "false" || in == "0") {
		out = false;
		return true;
	}
	return false;
}

auto Parser<std::string>::parse(std::string_view const in, std::string& out) -> bool {
	out = in;
	return true;
}
} // namespace le::tweak
