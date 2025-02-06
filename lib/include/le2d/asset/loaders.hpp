#pragma once
#include <djson/json.hpp>
#include <le2d/animation.hpp>
#include <le2d/asset/loader.hpp>
#include <le2d/asset/spir_v.hpp>
#include <le2d/font.hpp>
#include <le2d/texture.hpp>

namespace le {
class Context;

namespace asset {
class JsonLoader : public Loader<dj::Json> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<dj::Json>> final;
};

class SpirVLoader : public Loader<SpirV> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<SpirV>> final;
};

class FontLoader : public Loader<Font> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<Font>> final;
};

class TextureLoader : public Loader<Texture> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<Texture>> final;
};

class AnimationLoader : public Loader<Animation> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<Animation>> final;
};

class FlipbookLoader : public Loader<Flipbook> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<Flipbook>> final;
};
} // namespace asset
} // namespace le
