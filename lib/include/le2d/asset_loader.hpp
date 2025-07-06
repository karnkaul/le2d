#pragma once
#include <capo/buffer.hpp>
#include <le2d/anim/animation.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <gsl/pointers>
#include <memory>

namespace le {
class AssetLoader {
  public:
	/// \param data_loader Pointer to persistent IDataLoader instance.
	/// \param context Pointer to persistent Context instance.
	/// \param flags Asset load flags.
	explicit AssetLoader(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<IResourceFactory const*> factory);

	[[nodiscard]] auto get_factory() const -> IResourceFactory const& { return *m_factory; }
	[[nodiscard]] auto get_data_loader() const -> IDataLoader const& { return *m_data_loader; }

	[[nodiscard]] auto load_shader(std::string_view vertex_uri, std::string_view fragment_uri) const -> std::unique_ptr<IShader>;
	[[nodiscard]] auto load_font(std::string_view uri) const -> std::unique_ptr<IFont>;
	[[nodiscard]] auto load_tile_set(std::string_view uri) const -> std::unique_ptr<TileSet>;
	[[nodiscard]] auto load_texture(std::string_view uri) const -> std::unique_ptr<ITexture2>;
	[[nodiscard]] auto load_tile_sheet(std::string_view uri, std::string* out_texture_uri = {}) const -> std::unique_ptr<ITileSheet>;
	[[nodiscard]] auto load_audio_buffer(std::string_view uri) const -> std::unique_ptr<IAudioBuffer>;
	[[nodiscard]] auto load_transform_animation(std::string_view uri) const -> std::unique_ptr<anim::TransformAnimation>;
	[[nodiscard]] auto load_flipbook_animation(std::string_view uri) const -> std::unique_ptr<anim::FlipbookAnimation>;

  protected:
	static void on_success(std::string_view type, std::string_view uri);
	static void on_failure(std::string_view type, std::string_view uri);

  private:
	gsl::not_null<IDataLoader const*> m_data_loader;
	gsl::not_null<IResourceFactory const*> m_factory;
};
} // namespace le
