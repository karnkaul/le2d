#pragma once
#include <klib/assert.hpp>
#include <le2d/asset/asset.hpp>
#include <gsl/pointers>
#include <memory>
#include <string_view>
#include <typeindex>

namespace le {
namespace detail {
class IAssetTypeLoaderBase : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto type_name_impl() const -> std::string_view = 0;

	[[nodiscard]] virtual auto type_index() const -> std::type_index = 0;
	[[nodiscard]] virtual auto load_base(std::string_view uri) const -> std::unique_ptr<IAsset> = 0;
};
} // namespace detail

/// \brief Interface for a particular AssetType loader.
template <std::derived_from<IAsset> AssetTypeT>
class IAssetTypeLoader : public detail::IAssetTypeLoaderBase {
  public:
	/// \returns AssetType as a string, used for logs.
	[[nodiscard]] virtual auto type_name() const -> std::string_view = 0;

	/// \brief Primary customization point.
	/// \param uri URI to load from.
	/// \returns null on failure.
	[[nodiscard]] virtual auto load_asset(std::string_view uri) const -> std::unique_ptr<AssetTypeT> = 0;

  private:
	[[nodiscard]] auto type_name_impl() const -> std::string_view final { return type_name(); }
	[[nodiscard]] auto type_index() const -> std::type_index final { return std::type_index{typeid(AssetTypeT)}; }
	[[nodiscard]] auto load_base(std::string_view uri) const -> std::unique_ptr<IAsset> final { return load_asset(uri); }
};
} // namespace le
