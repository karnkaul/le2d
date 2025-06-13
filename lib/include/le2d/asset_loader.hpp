#pragma once
#include <capo/buffer.hpp>
#include <djson/json.hpp>
#include <klib/enum_ops.hpp>
#include <le2d/anim/animation.hpp>
#include <le2d/data_loader.hpp>
#include <le2d/font.hpp>
#include <gsl/pointers>

namespace le {
class Context;

enum class AssetLoadFlag : std::uint8_t {
	None = 0,
	Quiet = 1 << 0,
};

class AssetLoader {
  public:
	using Flag = AssetLoadFlag;

	/// \param data_loader Pointer to persistent IDataLoader instance.
	/// \param context Pointer to persistent Context instance.
	/// \param flags Asset load flags.
	explicit AssetLoader(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<Context const*> context, Flag flags = {});

	[[nodiscard]] auto load_bytes(std::string_view uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto load_spir_v(std::string_view uri) const -> std::vector<std::uint32_t>;
	[[nodiscard]] auto load_json(std::string_view uri) const -> dj::Json;
	[[nodiscard]] auto load_font(std::string_view uri) const -> Font;
	[[nodiscard]] auto load_tile_set(std::string_view uri) const -> TileSet;
	[[nodiscard]] auto load_texture(std::string_view uri) const -> Texture;
	[[nodiscard]] auto load_tile_sheet(std::string_view uri, std::string* out_texture_uri = {}) const -> TileSheet;
	[[nodiscard]] auto load_transform_animation(std::string_view uri) const -> anim::TransformAnimation;
	[[nodiscard]] auto load_flipbook_animation(std::string_view uri) const -> anim::FlipbookAnimation;
	[[nodiscard]] auto load_audio_buffer(std::string_view uri) const -> capo::Buffer;

  protected:
	[[nodiscard]] auto get_context() const -> Context const& { return *m_context; }
	[[nodiscard]] auto get_data_loader() const -> IDataLoader const& { return *m_data_loader; }

	void on_success(std::string_view type, std::string_view uri) const;
	void on_failure(std::string_view type, std::string_view uri) const;

  private:
	gsl::not_null<IDataLoader const*> m_data_loader;
	gsl::not_null<Context const*> m_context;
	Flag m_flags{};
};
} // namespace le

template <>
inline constexpr auto klib::enable_enum_ops_v<le::AssetLoadFlag> = true;
