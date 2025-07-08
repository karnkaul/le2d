#pragma once
#include <klib/assert.hpp>
#include <le2d/asset/asset_type_loader.hpp>
#include <gsl/pointers>
#include <memory>
#include <string_view>
#include <typeindex>
#include <unordered_map>

namespace le {
/// \brief Set of AssetTypeLoaders mapped to their corrseponding AssetTypes.
class AssetLoader {
  public:
	/// \brief Associate a loader with its type.
	/// \param loader concrete IAssetTypeLoader instance.
	void add_loader(std::unique_ptr<detail::IAssetTypeLoaderBase> loader);

	/// \tparam AssetTypeT Type of asset.
	/// \returns true if loader associated with AssetTypeT exists.
	template <std::derived_from<IAsset> AssetTypeT>
	[[nodiscard]] auto has_loader() const -> bool {
		return m_map.contains(std::type_index{typeid(AssetTypeT)});
	}

	/// \brief Load an asset given its URI.
	/// \tparam AssetTypeT Type of asset.
	/// \param uri URI to load from.
	/// \returns Asset instance if loaded, otherwise null.
	template <std::derived_from<IAsset> AssetTypeT>
	[[nodiscard]] auto load(std::string_view const uri) const -> std::unique_ptr<AssetTypeT> {
		auto ret = load_impl(std::type_index{typeid(AssetTypeT)}, uri);
		if (!ret) { return {}; }
		KLIB_ASSERT(dynamic_cast<AssetTypeT*>(ret.get()));
		return std::unique_ptr<AssetTypeT>{dynamic_cast<AssetTypeT*>(ret.release())};
	}

  private:
	[[nodiscard]] auto load_impl(std::type_index type, std::string_view uri) const -> std::unique_ptr<IAsset>;

	std::unordered_map<std::type_index, std::unique_ptr<detail::IAssetTypeLoaderBase>> m_map{};
};
} // namespace le
