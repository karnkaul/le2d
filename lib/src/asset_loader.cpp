#include <le2d/asset_loader.hpp>
#include <le2d/context.hpp>
#include <le2d/json_io.hpp>
#include <log.hpp>

namespace le {
AssetLoader::AssetLoader(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<Context const*> context)
	: m_data_loader(data_loader), m_context(context) {}

void AssetLoader::on_success(std::string_view const type, std::string_view const uri) { log.info("=='{}'== {} loaded", uri, type); }

void AssetLoader::on_failure(std::string_view const type, std::string_view const uri) { log.warn("'{}' Failed to load '{}'", type, uri); }

auto AssetLoader::load_shader_program(std::string_view const vertex_uri, std::string_view const fragment_uri) const -> ShaderProgram {
	static constexpr std::string_view type_v{"ShaderProgram"};

	auto const vertex_code = get_data_loader().load_spir_v(vertex_uri);
	if (vertex_code.empty()) {
		on_failure(type_v, vertex_uri);
		return {};
	}

	auto const fragment_code = get_data_loader().load_spir_v(fragment_uri);
	if (fragment_code.empty()) {
		on_failure(type_v, fragment_uri);
		return {};
	}

	auto const uri = std::format("{} + {}", vertex_uri, fragment_uri);
	auto ret = ShaderProgram{get_context().get_render_window().get_render_device().get_device(), vertex_code, fragment_code};
	if (!ret.is_loaded()) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_font(std::string_view const uri) const -> Font {
	static constexpr std::string_view type_v{"Font"};

	auto bytes = get_data_loader().load_bytes(uri);
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

	auto const json = get_data_loader().load_json(uri);
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

	auto const bytes = get_data_loader().load_bytes(uri);
	auto ret = get_context().create_texture();
	if (bytes.empty() || !ret.load_and_write(bytes)) {
		on_failure(type_v, uri);
		return ret;
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_tile_sheet(std::string_view const uri, std::string* out_texture_uri) const -> TileSheet {
	static constexpr std::string_view type_v{"TileSheet"};

	auto ret = get_context().create_tilesheet();
	auto const json = get_data_loader().load_json(uri);
	if (!is_json_type<TileSheet>(json)) {
		on_failure(type_v, uri);
		return ret;
	}

	auto const texture_uri = json["texture"].as_string_view();
	if (out_texture_uri) { *out_texture_uri = texture_uri; }
	auto const bytes = get_data_loader().load_bytes(texture_uri);
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
	auto const json = get_data_loader().load_json(uri);
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
	auto const json = get_data_loader().load_json(uri);
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

	auto const bytes = get_data_loader().load_bytes(uri);
	if (bytes.empty()) {
		on_failure(type_v, uri);
		return {};
	}

	auto const encoding = capo::guess_encoding(uri);
	auto ret = capo::Buffer{};
	if (!ret.decode_bytes(bytes, encoding)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}
} // namespace le
