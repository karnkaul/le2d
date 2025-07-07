#include <le2d/asset/asset_loaders.hpp>
#include <le2d/json_io.hpp>

namespace le {
namespace {
template <std::derived_from<IAsset> Type>
[[nodiscard]] auto json_to_asset(IDataLoader const& data_loader, std::string_view const uri) -> std::unique_ptr<Type> {
	auto const json = data_loader.load_json(uri);
	if (!is_json_type<Type>(json)) { return {}; }
	auto ret = std::make_unique<Type>();
	from_json(json, *ret);
	return ret;
}
} // namespace

auto ShaderLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<IShader> {
	auto const json = m_data_loader->load_json(uri);
	if (!json) { return {}; }
	auto const vert = m_data_loader->load_spir_v(json["vertex"].as_string_view());
	auto const frag = m_data_loader->load_spir_v(json["fragment"].as_string_view());
	if (vert.empty() || frag.empty()) { return {}; }
	auto ret = m_resource_factory->create_shader();
	if (!ret->load(vert, frag)) { return {}; }
	return ret;
}

auto FontLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<IFont> {
	auto bytes = m_data_loader->load_bytes(uri);
	if (bytes.empty()) { return {}; }
	auto ret = m_resource_factory->create_font();
	if (!ret->load_face(std::move(bytes))) { return {}; }
	return ret;
}

auto TextureLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<ITexture> {
	auto const bytes = m_data_loader->load_bytes(uri);
	if (bytes.empty()) { return {}; }
	auto ret = m_resource_factory->create_texture();
	if (!ret->load_and_write(bytes)) { return {}; }
	return ret;
}

auto TileSetLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<TileSet> { return json_to_asset<TileSet>(*m_data_loader, uri); }

auto TileSheetLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<ITileSheet> {
	auto const json = m_data_loader->load_json(uri);
	if (!is_json_type<ITileSheet>(json)) { return {}; }

	auto const texture_uri = json["texture"].as_string_view();
	auto const bytes = m_data_loader->load_bytes(texture_uri);
	if (bytes.empty()) { return {}; }

	auto ret = m_resource_factory->create_tilesheet();
	if (!ret->load_and_write(bytes)) { return {}; }

	auto const& tile_set_json = json["tile_set"];
	if (tile_set_json.is_string()) {
		auto tile_set = json_to_asset<TileSet>(*m_data_loader, tile_set_json.as_string_view());
		if (!tile_set) { return {}; }
		ret->tile_set = std::move(*tile_set);
	} else {
		from_json(tile_set_json, ret->tile_set);
	}

	return ret;
}

auto AudioBufferLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<IAudioBuffer> {
	auto const bytes = m_data_loader->load_bytes(uri);
	if (bytes.empty()) { return {}; }
	auto ret = m_resource_factory->create_audio_buffer();
	if (!ret->decode(bytes)) { return {}; }
	return ret;
}

auto TransformAnimationLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<TransformAnimation> {
	return json_to_asset<TransformAnimation>(*m_data_loader, uri);
}

auto FlipbookAnimationLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<FlipbookAnimation> {
	return json_to_asset<FlipbookAnimation>(*m_data_loader, uri);
}
} // namespace le
