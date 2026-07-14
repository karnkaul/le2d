#pragma once
#include "klib/ptr.hpp"
#include "le2d/asset/asset.hpp"
#include "le2d/context.hpp"
#include "le2d/uri.hpp"
#include <memory>
#include <unordered_map>

namespace le {
struct AssetView {
	gsl::not_null<Uri const*> uri;
	gsl::not_null<IAsset*> asset;
};

class AssetMap : public klib::Pinned {
  public:
	explicit AssetMap(gsl::not_null<Context*> context) : m_waiter(context->create_waiter()) {}

	void store_asset(Uri uri, std::unique_ptr<IAsset> asset);

	[[nodiscard]] auto contains_asset(Uri const& uri) const -> bool { return m_map.contains(uri); }

	template <std::derived_from<IAsset> AssetTypeT>
	[[nodiscard]] auto get_asset_if(Uri const& uri) const -> klib::Ptr<AssetTypeT> {
		auto const it = m_map.find(uri);
		if (it == m_map.end()) { return {}; }
		return dynamic_cast<AssetTypeT*>(it->second.get());
	}

	void erase_asset(Uri const& uri);
	void erase_assets(std::span<Uri const> uris);

	void clear_assets();

	[[nodiscard]] auto asset_count() const -> std::size_t { return m_map.size(); }

	void fill_asset_views(std::vector<AssetView>& out_views) const;
	[[nodiscard]] auto build_asset_views() const -> std::vector<AssetView>;

  private:
	std::unordered_map<Uri, std::unique_ptr<IAsset>, Uri::Hasher> m_map{};

	Context::Waiter m_waiter;
};
} // namespace le
