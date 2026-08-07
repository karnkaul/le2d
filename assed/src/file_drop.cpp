#include "file_drop.hpp"
#include <algorithm>
#include <array>
#include <filesystem>

namespace le::assed {
namespace fs = std::filesystem;

namespace {
[[nodiscard]] constexpr auto contains(std::span<char const* const> values, std::string_view const value) -> bool {
	return std::ranges::find(values, value) != values.end();
}
} // namespace

auto FileDrop::create(FileDataLoader const& loader, std::string_view const path) -> FileDrop {
	auto ret = FileDrop{.uri = loader.get_uri(path)};
	if (!ret) { return ret; }

	if (fs::is_directory(path)) {
		ret.type = Type::Directory;
		return ret;
	}

	ret.type = Type::Unknown;
	if (!fs::is_regular_file(path)) { return ret; }

	ret.extension = fs::path{ret.uri.get_string()}.extension().string();

	static constexpr auto json_extensions_v = std::array{".json", ".jsonc"};
	if (contains(json_extensions_v, ret.extension)) {
		ret.type = Type::Json;
		ret.json_type = loader.get_json_type_name(ret.uri.get_string());
		return ret;
	}

	static constexpr auto image_extensions_v = std::array{".png", ".jpg", ".jpeg", ".bmp", ".tga"};
	if (contains(image_extensions_v, ret.extension)) {
		ret.type = Type::Image;
		return ret;
	}

	static constexpr auto font_extensions_v = std::array{".ttf", ".otf"};
	if (contains(font_extensions_v, ret.extension)) {
		ret.type = Type::Font;
		return ret;
	}

	return ret;
}
} // namespace le::assed
