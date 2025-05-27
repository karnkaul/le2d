#include <djson/json.hpp>
#include <le2d/data_loader.hpp>

namespace le {
auto IDataLoader::load_json(dj::Json& out, std::string_view const uri) const -> bool {
	auto str = std::string{};
	if (!load_string(str, uri)) { return false; }
	auto result = dj::Json::parse(str, dj::ParseMode::Jsonc);
	if (!result) { return false; }
	out = std::move(*result);
	return true;
}

auto IDataLoader::get_json_type_name(std::string_view const uri) const -> std::string {
	auto json = dj::Json{};
	if (!load_json(json, uri)) { return {}; }
	return json["type_name"].as<std::string>();
}
} // namespace le
