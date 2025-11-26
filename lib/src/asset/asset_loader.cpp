#include "le2d/asset/asset_loader.hpp"
#include <klib/log.hpp>

namespace le {
namespace {
auto const log = klib::TaggedLogger{"AssetLoader"};
} // namespace

void AssetLoader::add_loader(std::unique_ptr<detail::IAssetTypeLoaderBase> loader) {
	if (!loader) { return; }
	auto const index = loader->type_index();
	m_map.insert_or_assign(index, std::move(loader));
}

auto AssetLoader::load_impl(std::type_info const& type, std::string_view const uri) const -> std::unique_ptr<IAsset> {
	auto const it = m_map.find(type);
	if (it == m_map.end()) {
		log.error("'{}' Missing AssetTypeLoader for '{}'", uri, klib::demangled_name(type));
		return {};
	}

	auto const& loader = *it->second;
	auto ret = loader.load_base(uri);
	if (!ret) {
		log.warn("'{}' Failed to load {}", uri, loader.type_name());
		return {};
	}

	log.info("=='{}' {} loaded==", uri, loader.type_name());
	return ret;
}
} // namespace le
