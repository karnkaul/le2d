#pragma once
#include <capo/buffer.hpp>
#include <le2d/anim/animation.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <gsl/pointers>
#include <memory>

namespace le {
/// \brief Utility for loading assets given URIs to their data.
///
/// All load*() functions return null on failure.
/// Can be customized by subclassing.
class AssetLoaderOld : public klib::Polymorphic {
  public:
	/// \param data_loader Pointer to persistent IDataLoader instance.
	/// \param factory Pointer to persistent IResourceFactory instance.
	explicit AssetLoaderOld(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<IResourceFactory const*> resource_factory);

	[[nodiscard]] auto get_data_loader() const -> IDataLoader const& { return *m_data_loader; }
	[[nodiscard]] auto get_resource_factory() const -> IResourceFactory const& { return *m_resource_factory; }

	[[nodiscard]] auto load_tile_set(std::string_view uri) const -> std::unique_ptr<TileSet>;
	[[nodiscard]] auto load_transform_animation(std::string_view uri) const -> std::unique_ptr<TransformAnimation>;
	[[nodiscard]] auto load_flipbook_animation(std::string_view uri) const -> std::unique_ptr<FlipbookAnimation>;

	[[nodiscard]] auto load_shader(std::string_view vertex_uri, std::string_view fragment_uri) const -> std::unique_ptr<IShader>;
	[[nodiscard]] auto load_font(std::string_view uri) const -> std::unique_ptr<IFont>;
	[[nodiscard]] auto load_texture(std::string_view uri) const -> std::unique_ptr<ITexture>;
	[[nodiscard]] auto load_tile_sheet(std::string_view uri, std::string* out_texture_uri = {}) const -> std::unique_ptr<ITileSheet>;
	[[nodiscard]] auto load_audio_buffer(std::string_view uri) const -> std::unique_ptr<IAudioBuffer>;

  protected:
	static void on_success(std::string_view type, std::string_view uri);
	static void on_failure(std::string_view type, std::string_view uri);

  private:
	gsl::not_null<IDataLoader const*> m_data_loader;
	gsl::not_null<IResourceFactory const*> m_resource_factory;
};
} // namespace le
