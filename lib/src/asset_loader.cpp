#include <le2d/asset_loader.hpp>
#include <le2d/context.hpp>
#include <le2d/json_io.hpp>
#include <log.hpp>

namespace le {
void AssetLoader::on_success(std::string_view const type, std::string_view const uri) const {
	if ((m_flags & Flag::Quiet) == Flag::Quiet) { return; }
	log.info("=='{}'== {} loaded", uri, type);
}

void AssetLoader::on_failure(std::string_view const type, std::string_view const uri) const {
	if ((m_flags & Flag::Quiet) == Flag::Quiet) { return; }
	log.warn("'{}' Failed to load '{}'", type, uri);
}

auto AssetLoader::load_bytes(std::string_view const uri) const -> std::vector<std::byte> {
	static constexpr std::string_view type_v{"Bytes"};

	auto ret = std::vector<std::byte>{};
	if (!get_context().get_data_loader().load_bytes(ret, uri)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_spir_v(std::string_view const uri) const -> std::vector<std::uint32_t> {
	static constexpr std::string_view type_v{"SpirV"};

	auto ret = std::vector<std::uint32_t>{};
	if (!get_context().get_data_loader().load_spirv(ret, uri)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_json(std::string_view const uri) const -> dj::Json {
	static constexpr std::string_view type_v{"Json"};

	auto ret = dj::Json{};
	if (!get_context().get_data_loader().load_json(ret, uri)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_font(std::string_view const uri) const -> Font {
	static constexpr std::string_view type_v{"Font"};

	auto bytes = AssetLoader{m_context, Flag::Quiet}.load_bytes(uri);
	if (bytes.empty()) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = get_context().create_font(std::move(bytes));
	if (!ret.is_loaded()) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_tile_set(std::string_view const uri) const -> TileSet {
	static constexpr std::string_view type_v{"TileSet"};

	auto const json = AssetLoader{m_context, Flag::Quiet}.load_json(uri);
	if (!is_json_type<TileSet>(json)) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = TileSet{};
	from_json(json, ret);
	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_texture(std::string_view const uri) const -> Texture {
	static constexpr std::string_view type_v{"Texture"};

	auto const bytes = AssetLoader{m_context, Flag::Quiet}.load_bytes(uri);
	auto ret = get_context().create_texture();
	if (bytes.empty() || !ret.load_and_write(bytes)) {
		on_failure(type_v, uri);
		return ret;
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_tile_sheet(std::string_view uri, std::string* out_texture_uri) const -> TileSheet {
	static constexpr std::string_view type_v{"TileSheet"};

	auto const quiet_loader = AssetLoader{m_context, Flag::Quiet};
	auto ret = get_context().create_tilesheet();
	auto const json = quiet_loader.load_json(uri);
	if (!is_json_type<TileSheet>(json)) {
		on_failure(type_v, uri);
		return ret;
	}

	auto const texture_uri = json["texture"].as_string_view();
	if (out_texture_uri) { *out_texture_uri = texture_uri; }
	auto const bytes = quiet_loader.load_bytes(texture_uri);
	if (bytes.empty() || !ret.load_and_write(bytes)) {
		on_failure(type_v, uri);
		return ret;
	}

	auto const& tile_set_json = json["tile_set"];
	if (tile_set_json.is_string()) {
		ret.tile_set = load_tile_set(json["tile_set"].as_string_view());
	} else {
		from_json(tile_set_json, ret.tile_set);
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_transform_animation(std::string_view const uri) const -> anim::TransformAnimation {
	static constexpr std::string_view type_v{"TransformAnimation"};

	auto ret = anim::TransformAnimation{};
	auto const json = AssetLoader{m_context, Flag::Quiet}.load_json(uri);
	if (!is_json_type(json, ret)) {
		on_failure(type_v, uri);
		return {};
	}

	from_json(json, ret);
	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_flipbook_animation(std::string_view const uri) const -> anim::FlipbookAnimation {
	static constexpr std::string_view type_v{"FlipbookAnimation"};

	auto ret = anim::FlipbookAnimation{};
	auto const json = AssetLoader{m_context, Flag::Quiet}.load_json(uri);
	if (!is_json_type(json, ret)) {
		on_failure(type_v, uri);
		return {};
	}

	from_json(json, ret);
	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_audio_buffer(std::string_view const uri) const -> capo::Buffer {
	static constexpr std::string_view type_v{"AudioBuffer"};

	auto const bytes = AssetLoader{m_context, Flag::Quiet}.load_bytes(uri);
	if (bytes.empty()) {
		on_failure(type_v, uri);
		return {};
	}

	auto const encoding = capo::guess_encoding(std::string{uri}.c_str());
	auto ret = capo::Buffer{};
	if (!ret.decode_bytes(bytes, encoding)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}
} // namespace le
