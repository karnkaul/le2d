#pragma once
#include <le2d/asset/wrap.hpp>
#include <le2d/uri.hpp>
#include <gsl/pointers>
#include <memory>

namespace le {
class Context;

namespace asset {
class ILoader : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto load_base(Uri const& uri) const -> std::unique_ptr<Base> = 0;
};

template <typename AssetT>
class Loader : public ILoader {
  public:
	using value_type = AssetT;

	explicit Loader(gsl::not_null<Context const*> context) : m_context(context) {}

	[[nodiscard]] virtual auto load(Uri const& uri) const -> std::unique_ptr<Wrap<AssetT>> = 0;

	[[nodiscard]] auto load_base(Uri const& uri) const -> std::unique_ptr<Base> final { return load(uri); }

  protected:
	gsl::not_null<Context const*> m_context;
};
} // namespace asset
} // namespace le
