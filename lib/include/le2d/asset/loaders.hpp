#pragma once
#include <le2d/asset/loader.hpp>
#include <le2d/font.hpp>

namespace le {
class Context;

namespace asset {
class FontLoader : public Loader<Font> {
  public:
	explicit FontLoader(gsl::not_null<Context const*> context) : m_context(context) {}

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<Font>> final;

  private:
	gsl::not_null<Context const*> m_context;
};
} // namespace asset
} // namespace le
