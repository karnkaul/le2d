#pragma once
#include <le2d/anim/animation.hpp>
#include <le2d/asset/asset_loader.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/resource/audio_buffer.hpp>
#include <le2d/resource/font.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <le2d/resource/shader.hpp>
#include <le2d/resource/texture.hpp>
#include <le2d/tile/tile_set.hpp>

namespace le {
template <std::derived_from<IAsset> AssetTypeT>
class IAssetTypeLoaderCommon : public IAssetTypeLoader<AssetTypeT> {
  public:
	using BaseType = IAssetTypeLoaderCommon;

	explicit IAssetTypeLoaderCommon(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<IResourceFactory const*> resource_factory)
		: m_data_loader(data_loader), m_resource_factory(resource_factory) {}

  protected:
	gsl::not_null<IDataLoader const*> m_data_loader;
	gsl::not_null<IResourceFactory const*> m_resource_factory;
};

class ShaderLoader : public IAssetTypeLoaderCommon<IShader> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<IShader> final;
};

class FontLoader : public IAssetTypeLoaderCommon<IFont> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<IFont> final;
};

class TextureLoader : public IAssetTypeLoaderCommon<ITexture> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<ITexture> final;
};

class TileSetLoader : public IAssetTypeLoaderCommon<TileSet> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<TileSet> final;
};

class TileSheetLoader : public IAssetTypeLoaderCommon<ITileSheet> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<ITileSheet> final;
};

class AudioBufferLoader : public IAssetTypeLoaderCommon<IAudioBuffer> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<IAudioBuffer> final;
};

class TransformAnimationLoader : public IAssetTypeLoaderCommon<TransformAnimation> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<TransformAnimation> final;
};

class FlipbookAnimationLoader : public IAssetTypeLoaderCommon<FlipbookAnimation> {
  public:
	using BaseType::BaseType;

	[[nodiscard]] auto load_asset(std::string_view uri) const -> std::unique_ptr<FlipbookAnimation> final;
};
} // namespace le
