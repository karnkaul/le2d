#pragma once
#include <le2d/anim/animation.hpp>
#include <le2d/asset/asset_loader.hpp>
#include <le2d/resource/audio_buffer.hpp>
#include <le2d/resource/font.hpp>
#include <le2d/resource/shader.hpp>
#include <le2d/resource/texture.hpp>
#include <le2d/tile/tile_set.hpp>

namespace le {
class ShaderLoader : public IAssetLoader<IShader> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "Shader"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<IShader> final;
};

class FontLoader : public IAssetLoader<IFont> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "Font"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<IFont> final;
};

class TextureLoader : public IAssetLoader<ITexture> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "Texture"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<ITexture> final;
};

class TileSetLoader : public IAssetLoader<TileSet> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "TileSet"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<TileSet> final;
};

class TileSheetLoader : public IAssetLoader<ITileSheet> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "TileSheet"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<ITileSheet> final;
};

class AudioBufferLoader : public IAssetLoader<IAudioBuffer> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "AudioBuffer"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<IAudioBuffer> final;
};

class TransformAnimationLoader : public IAssetLoader<TransformAnimation> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "TransformAnimation"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<TransformAnimation> final;
};

class FlipbookAnimationLoader : public IAssetLoader<FlipbookAnimation> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto type_name() const -> std::string_view final { return "FlipbookAnimation"; }

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<FlipbookAnimation> final;
};
} // namespace le
