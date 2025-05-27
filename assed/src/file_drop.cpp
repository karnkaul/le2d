#include <file_drop.hpp>
#include <le2d/json_io.hpp>
#include <algorithm>
#include <array>
#include <filesystem>

namespace le::assed {
namespace fs = std::filesystem;

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
	if (ret.extension == ".json") {
		ret.type = Type::Json;
		ret.json_type = loader.get_json_type_name(ret.uri.get_string());
		return ret;
	}

	static constexpr auto image_extensions_v = std::array{".png", ".jpg", ".jpeg", ".bmp", ".tga"};
	if (std::ranges::find(image_extensions_v, ret.extension) != image_extensions_v.end()) {
		ret.type = Type::Image;
		return ret;
	}

	static constexpr auto font_extensions_v = std::array{".ttf", ".otf"};
	if (std::ranges::find(font_extensions_v, ret.extension) != font_extensions_v.end()) {
		ret.type = Type::Font;
		return ret;
	}

	return ret;
}
} // namespace le::assed
