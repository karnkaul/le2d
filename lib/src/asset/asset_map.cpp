#include "le2d/asset/asset_map.hpp"

namespace le {
void AssetMap::store_asset(Uri uri, std::unique_ptr<IAsset> asset) {
	if (!asset) { return; }
	m_map.insert_or_assign(std::move(uri), std::move(asset));
}

void AssetMap::erase_asset(Uri const& uri) {
	auto const it = m_map.find(uri);
	if (it == m_map.end()) { return; }

	m_waiter.get_context()->wait_idle();
	m_map.erase(it);
}

void AssetMap::erase_assets(std::span<Uri const> uris) {
	m_waiter.get_context()->wait_idle();
	for (auto const& uri : uris) { m_map.erase(uri); }
}

void AssetMap::clear_assets() {
	if (m_map.empty()) { return; }

	m_waiter.get_context()->wait_idle();
	m_map.clear();
}

void AssetMap::fill_asset_views(std::vector<AssetView>& out_views) const {
	if (m_map.empty()) { return; }
	out_views.reserve(out_views.size() + m_map.size());
	for (auto const& [uri, asset] : m_map) { out_views.push_back(AssetView{.uri = &uri, .asset = asset.get()}); }
}

auto AssetMap::build_asset_views() const -> std::vector<AssetView> {
	auto ret = std::vector<AssetView>{};
	fill_asset_views(ret);
	return ret;
}
} // namespace le
