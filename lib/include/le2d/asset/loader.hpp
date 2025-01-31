#pragma once
#include <le2d/asset/wrap.hpp>
#include <le2d/uri.hpp>
#include <memory>
#include <typeindex>

namespace le::asset {
class ILoader : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_type() const -> std::type_index = 0;
	[[nodiscard]] virtual auto load_base(Uri const& uri) const -> std::unique_ptr<Base> = 0;
};

template <typename AssetT>
class Loader : public ILoader {
  public:
	[[nodiscard]] virtual auto load(Uri const& uri) const -> std::unique_ptr<Wrap<AssetT>> = 0;

	[[nodiscard]] auto get_type() const -> std::type_index final { return typeid(AssetT); }
	[[nodiscard]] auto load_base(Uri const& uri) const -> std::unique_ptr<Base> final { return load(uri); }
};
} // namespace le::asset
