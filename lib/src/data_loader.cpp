#include <djson/json.hpp>
#include <le2d/data_loader.hpp>

namespace le {
auto IDataLoader::load_json(dj::Json& out, Uri const& uri) const -> bool {
	auto str = std::string{};
	if (!load_string(str, uri)) { return false; }
	out = dj::Json::parse(str);
	return true;
}

auto IDataLoader::get_json_type_name(Uri const& uri) const -> std::string {
	auto json = dj::Json{};
	if (!load_json(json, uri)) { return {}; }
	return json["type_name"].as<std::string>();
}
} // namespace le
