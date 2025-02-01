#pragma once
#include <le2d/asset/wrap.hpp>
#include <le2d/uri.hpp>
#include <memory>

namespace le::asset {
class ILoader : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto load_base(Uri const& uri) const -> std::unique_ptr<Base> = 0;
};

template <typename AssetT>
class Loader : public ILoader {
  public:
	using type = AssetT;

	[[nodiscard]] virtual auto load(Uri const& uri) const -> std::unique_ptr<Wrap<AssetT>> = 0;

	[[nodiscard]] auto load_base(Uri const& uri) const -> std::unique_ptr<Base> final { return load(uri); }
};
} // namespace le::asset
