#include "le2d/tweak/parser.hpp"
#include "kvf/util.hpp"

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

auto Parser<kvf::Color>::parse(std::string_view const in, kvf::Color& out) -> bool {
	auto const result = kvf::util::color_from_hex(in);
	if (!result) { return false; }
	out = *result;
	return true;
}

auto Parser<kvf::Color>::to_string(kvf::Color const color) -> std::string { return kvf::util::to_hex_string(color); }
} // namespace le::tweak
