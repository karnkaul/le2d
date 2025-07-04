#pragma once
#include <capo/buffer.hpp>
#include <djson/json.hpp>
#include <le2d/anim/animation.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/font.hpp>
#include <le2d/shader_program.hpp>
#include <gsl/pointers>

namespace le {
class Context;

class AssetLoader {
  public:
	/// \param data_loader Pointer to persistent IDataLoader instance.
	/// \param context Pointer to persistent Context instance.
	/// \param flags Asset load flags.
	explicit AssetLoader(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<Context const*> context);

	[[nodiscard]] auto get_context() const -> Context const& { return *m_context; }
	[[nodiscard]] auto get_data_loader() const -> IDataLoader const& { return *m_data_loader; }

	[[nodiscard]] auto load_shader_program(std::string_view vertex_uri, std::string_view fragment_uri) const -> ShaderProgram;
	[[nodiscard]] auto load_string(std::string_view uri) const -> std::string;
	[[nodiscard]] auto load_font(std::string_view uri) const -> Font;
	[[nodiscard]] auto load_tile_set(std::string_view uri) const -> TileSet;
	[[nodiscard]] auto load_texture(std::string_view uri) const -> Texture;
	[[nodiscard]] auto load_tile_sheet(std::string_view uri, std::string* out_texture_uri = {}) const -> TileSheet;
	[[nodiscard]] auto load_transform_animation(std::string_view uri) const -> anim::TransformAnimation;
	[[nodiscard]] auto load_flipbook_animation(std::string_view uri) const -> anim::FlipbookAnimation;
	[[nodiscard]] auto load_audio_buffer(std::string_view uri) const -> capo::Buffer;

  protected:
	static void on_success(std::string_view type, std::string_view uri);
	static void on_failure(std::string_view type, std::string_view uri);

  private:
	gsl::not_null<IDataLoader const*> m_data_loader;
	gsl::not_null<Context const*> m_context;
};
} // namespace le
