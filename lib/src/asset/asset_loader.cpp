#include <le2d/asset/asset_loader.hpp>
#include <le2d/json_io.hpp>
#include <log.hpp>

namespace le {
AssetLoader::AssetLoader(gsl::not_null<IDataLoader const*> data_loader, gsl::not_null<IResourceFactory const*> resource_factory)
	: m_data_loader(data_loader), m_resource_factory(resource_factory) {}

void AssetLoader::on_success(std::string_view const type, std::string_view const uri) { log.info("=='{}'== {} loaded", uri, type); }

void AssetLoader::on_failure(std::string_view const type, std::string_view const uri) { log.warn("'{}' Failed to load '{}'", type, uri); }

auto AssetLoader::load_tile_set(std::string_view const uri) const -> std::unique_ptr<TileSet> {
	static constexpr std::string_view type_v{"TileSet"};

	auto const json = get_data_loader().load_json(uri);
	if (!is_json_type<TileSet>(json)) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = std::make_unique<TileSet>();
	from_json(json, *ret);
	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_transform_animation(std::string_view const uri) const -> std::unique_ptr<TransformAnimation> {
	static constexpr std::string_view type_v{"TransformAnimation"};

	auto const json = get_data_loader().load_json(uri);
	if (!is_json_type<TransformAnimation>(json)) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = std::make_unique<TransformAnimation>();
	from_json(json, *ret);
	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_flipbook_animation(std::string_view const uri) const -> std::unique_ptr<FlipbookAnimation> {
	static constexpr std::string_view type_v{"FlipbookAnimation"};

	auto const json = get_data_loader().load_json(uri);
	if (!is_json_type<FlipbookAnimation>(json)) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = std::make_unique<FlipbookAnimation>();
	from_json(json, *ret);
	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_shader(std::string_view const vertex_uri, std::string_view const fragment_uri) const -> std::unique_ptr<IShader> {
	static constexpr std::string_view type_v{"Shader"};

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
	auto ret = m_resource_factory->create_shader();
	if (!ret->load(vertex_code, fragment_code)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_font(std::string_view const uri) const -> std::unique_ptr<IFont> {
	static constexpr std::string_view type_v{"Font"};

	auto bytes = get_data_loader().load_bytes(uri);
	if (bytes.empty()) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = m_resource_factory->create_font();
	if (!ret->load_face(std::move(bytes))) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_texture(std::string_view const uri) const -> std::unique_ptr<ITexture> {
	static constexpr std::string_view type_v{"Texture"};

	auto const bytes = get_data_loader().load_bytes(uri);
	if (bytes.empty()) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = m_resource_factory->create_texture();
	if (!ret->load_and_write(bytes)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_tile_sheet(std::string_view const uri, std::string* out_texture_uri) const -> std::unique_ptr<ITileSheet> {
	static constexpr std::string_view type_v{"TileSheet"};

	auto const json = get_data_loader().load_json(uri);
	if (!is_json_type<ITileSheet>(json)) {
		on_failure(type_v, uri);
		return {};
	}

	auto const texture_uri = json["texture"].as_string_view();
	if (out_texture_uri) { *out_texture_uri = texture_uri; }
	auto const bytes = get_data_loader().load_bytes(texture_uri);
	if (bytes.empty()) {
		on_failure(type_v, uri);
		return {};
	}

	auto ret = m_resource_factory->create_tilesheet();
	if (!ret->load_and_write(bytes)) {
		on_failure(type_v, uri);
		return {};
	}

	auto const& tile_set_json = json["tile_set"];
	if (tile_set_json.is_string()) {
		auto tile_set = load_tile_set(tile_set_json.as_string_view());
		if (!tile_set) {
			on_failure(type_v, uri);
			return {};
		}
		ret->tile_set = std::move(*tile_set);
	} else {
		from_json(tile_set_json, ret->tile_set);
	}

	on_success(type_v, uri);
	return ret;
}

auto AssetLoader::load_audio_buffer(std::string_view const uri) const -> std::unique_ptr<IAudioBuffer> {
	static constexpr std::string_view type_v{"AudioBuffer"};

	auto const bytes = get_data_loader().load_bytes(uri);
	if (bytes.empty()) {
		on_failure(type_v, uri);
		return {};
	}

	auto const encoding = capo::guess_encoding(uri);
	auto ret = m_resource_factory->create_audio_buffer();
	if (!ret->decode(bytes, encoding)) {
		on_failure(type_v, uri);
		return {};
	}

	on_success(type_v, uri);
	return ret;
}
} // namespace le
