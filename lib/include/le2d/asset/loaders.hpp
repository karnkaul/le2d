#pragma once
#include <capo/pcm.hpp>
#include <djson/json.hpp>
#include <le2d/anim/animation.hpp>
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

class TileSetLoader : public Loader<TileSet> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<TileSet>> final;
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

class TransformAnimationLoader : public Loader<anim::Animation<Transform>> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<anim::Animation<Transform>>> final;
};

class TileAnimationLoader : public Loader<anim::Animation<TileId>> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<anim::Animation<TileId>>> final;
};

class PcmLoader : public Loader<capo::Pcm> {
  public:
	using Loader::Loader;

	[[nodiscard]] auto load(Uri const& uri) const -> std::unique_ptr<Wrap<capo::Pcm>> final;
};
} // namespace asset
} // namespace le
