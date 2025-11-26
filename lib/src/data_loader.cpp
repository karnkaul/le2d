#include "le2d/data_loader.hpp"
#include <djson/json.hpp>

namespace le {
auto IDataLoader::load_bytes(std::string_view const uri) const -> std::vector<std::byte> {
	auto ret = std::vector<std::byte>{};
	if (try_load_bytes(ret, uri)) { return ret; }
	return {};
}

auto IDataLoader::load_spir_v(std::string_view const uri) const -> std::vector<std::uint32_t> {
	auto ret = std::vector<std::uint32_t>{};
	if (try_load_spirv(ret, uri)) { return ret; }
	return {};
}

auto IDataLoader::load_string(std::string_view const uri) const -> std::string {
	auto ret = std::string{};
	if (try_load_string(ret, uri)) { return ret; }
	return {};
}

auto IDataLoader::try_load_json(dj::Json& out, std::string_view const uri) const -> bool {
	auto str = std::string{};
	if (!try_load_string(str, uri)) { return false; }
	auto result = dj::Json::parse(str, dj::ParseMode::Jsonc);
	if (!result) { return false; }
	out = std::move(*result);
	return true;
}

auto IDataLoader::load_json(std::string_view const uri) const -> dj::Json {
	auto json = dj::Json{};
	if (!try_load_json(json, uri)) { return {}; }
	return json;
}

auto IDataLoader::get_json_type_name(std::string_view const uri) const -> std::string {
	auto json = dj::Json{};
	if (!try_load_json(json, uri)) { return {}; }
	return json["type_name"].as<std::string>();
}
} // namespace le
