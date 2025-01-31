#pragma once
#include <le2d/asset/loader.hpp>
#include <gsl/pointers>

namespace le {
class Context;
class Font;
class Texture;

namespace asset {
class FontLoader : public Loader<Font> {
  public:
	explicit FontLoader(gsl::not_null<Context const*> context) : m_context(context) {}

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<Font>> final;

  private:
	Context const* m_context;
};

class TextureLoader : public Loader<Texture> {
  public:
	explicit TextureLoader(gsl::not_null<Context const*> context) : m_context(context) {}

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<Texture>> final;

  private:
	Context const* m_context;
};
} // namespace asset
} // namespace le
