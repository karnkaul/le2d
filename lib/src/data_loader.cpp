#include <djson/json.hpp>
#include <le2d/data_loader.hpp>

namespace le {
auto IDataLoader::load_json(dj::Json& out, Uri const& uri) const -> bool {
	auto str = std::string{};
	if (!load_string(str, uri)) { return false; }
	out = dj::Json::parse(str);
	return true;
}
} // namespace le
