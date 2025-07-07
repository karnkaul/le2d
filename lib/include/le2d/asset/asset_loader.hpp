#pragma once
#include <klib/assert.hpp>
#include <le2d/asset/asset.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <gsl/pointers>
#include <memory>
#include <string_view>
#include <typeindex>
#include <unordered_map>

namespace le {
namespace detail {
class IAssetLoaderBase : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto type_name() const -> std::string_view = 0;

	[[nodiscard]] virtual auto type_index() const -> std::type_index = 0;
	[[nodiscard]] virtual auto load_base(std::string_view uri) const -> std::unique_ptr<IAsset> = 0;
};
} // namespace detail

/// \brief Customizable loader for a particular asset type.
template <std::derived_from<IAsset> AssetTypeT>
class IAssetLoader : public detail::IAssetLoaderBase {
  public:
	using BaseType = IAssetLoader;

	explicit IAssetLoader(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<IResourceFactory const*> resource_factory)
		: m_data_loader(data_loader), m_resource_factory(resource_factory) {}

	[[nodiscard]] virtual auto load_asset(std::string_view uri) const -> std::unique_ptr<AssetTypeT> = 0;

  private:
	[[nodiscard]] auto type_index() const -> std::type_index final { return std::type_index{typeid(AssetTypeT)}; }
	[[nodiscard]] auto load_base(std::string_view uri) const -> std::unique_ptr<IAsset> final { return load_asset(uri); }

  protected:
	gsl::not_null<IDataLoader const*> m_data_loader;
	gsl::not_null<IResourceFactory const*> m_resource_factory;
};

class AssetLoader {
  public:
	void add_loader(std::unique_ptr<detail::IAssetLoaderBase> loader);

	template <std::derived_from<IAsset> AssetTypeT>
	[[nodiscard]] auto has_loader() const -> bool {
		return m_map.contains(std::type_index{typeid(AssetTypeT)});
	}

	template <std::derived_from<IAsset> AssetTypeT>
	[[nodiscard]] auto load(std::string_view const uri) const -> std::unique_ptr<AssetTypeT> {
		auto ret = load_impl(std::type_index{typeid(AssetTypeT)}, uri);
		if (!ret) { return {}; }
		KLIB_ASSERT(dynamic_cast<AssetTypeT*>(ret.get()));
		return std::unique_ptr<AssetTypeT>{dynamic_cast<AssetTypeT*>(ret.release())};
	}

  private:
	[[nodiscard]] auto load_impl(std::type_index type, std::string_view uri) const -> std::unique_ptr<IAsset>;

	std::unordered_map<std::type_index, std::unique_ptr<detail::IAssetLoaderBase>> m_map{};
};
} // namespace le
