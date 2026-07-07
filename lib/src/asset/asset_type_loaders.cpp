#include "le2d/asset/asset_type_loaders.hpp"
#include "kvf/image_bitmap.hpp"
#include "le2d/json_io.hpp"

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

constexpr auto to_address_mode(std::string_view const in) {
	if (in == "repeat") { return vk::SamplerAddressMode::eRepeat; }
	if (in == "clamp_border") { return vk::SamplerAddressMode::eClampToBorder; }
	return vk::SamplerAddressMode::eClampToEdge;
}

constexpr auto to_filter(std::string_view const in) {
	if (in == "nearest") { return vk::Filter::eNearest; }
	return vk::Filter::eLinear;
}

constexpr auto to_border_color(std::string_view const in) {
	if (in == "black") { return vk::BorderColor::eFloatOpaqueBlack; }
	if (in == "white") { return vk::BorderColor::eFloatOpaqueWhite; }
	return vk::BorderColor::eFloatTransparentBlack;
}

auto to_texture_sampler(dj::Json const& json) -> TextureSampler {
	auto ret = TextureSampler{};
	if (auto const& wrap = json["wrap"]) { ret.wrap = to_address_mode(wrap.as_string_view()); }
	if (auto const& filter = json["filter"]) { ret.filter = to_filter(filter.as_string_view()); }
	if (auto const& border = json["border"]) { ret.border = to_border_color(border.as_string_view()); }
	return ret;
}

struct TextureData {
	kvf::ImageBitmap image{};
	TextureSampler sampler{};
};

[[nodiscard]] auto to_texture_data(IDataLoader const& data_loader, std::string_view const uri) -> TextureData {
	auto ret = TextureData{};
	auto image_uri = uri;

	if (uri.ends_with(".json") || uri.ends_with(".jsonc")) {
		auto const json = data_loader.load_json(uri);
		if (!json) { return ret; }

		image_uri = json["image"].as_string_view();
		ret.sampler = to_texture_sampler(json);
	}

	auto const image_bytes = data_loader.load_bytes(image_uri);
	ret.image.decompress(image_bytes);

	return ret;
}
} // namespace

auto ShaderLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<IShader> {
	auto const json = m_data_loader->load_json(uri);
	if (!is_json_type<IShader>(json)) { return {}; }
	auto const vert = m_data_loader->load_spir_v(json["vertex"].as_string_view());
	auto const frag = m_data_loader->load_spir_v(json["fragment"].as_string_view());
	if (vert.empty() || frag.empty()) { return {}; }
	return m_resource_factory->create_shader(vert, frag);
}

auto FontLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<IFont> {
	auto bytes = m_data_loader->load_bytes(uri);
	if (bytes.empty()) { return {}; }
	return m_resource_factory->create_font(std::move(bytes));
}

auto TextureLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<ITexture> {
	auto const data = to_texture_data(*m_data_loader, uri);
	if (!data.image.is_loaded()) { return {}; }
	return m_resource_factory->create_texture(data.image.bitmap(), data.sampler);
}

auto TileSetLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<TileSet> { return json_to_asset<TileSet>(*m_data_loader, uri); }

auto TileSheetLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<ITileSheet> {
	auto const json = m_data_loader->load_json(uri);
	if (!is_json_type<ITileSheet>(json)) { return {}; }

	auto const data = to_texture_data(*m_data_loader, json["texture"].as_string_view());
	if (!data.image.is_loaded()) { return {}; }

	auto ret = m_resource_factory->create_tilesheet(data.image.bitmap(), data.sampler);

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
	return m_resource_factory->create_audio_buffer(bytes);
}

auto TransformAnimationLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<TransformAnimation> {
	return json_to_asset<TransformAnimation>(*m_data_loader, uri);
}

auto FlipbookAnimationLoader::load_asset(std::string_view const uri) const -> std::unique_ptr<FlipbookAnimation> {
	return json_to_asset<FlipbookAnimation>(*m_data_loader, uri);
}
} // namespace le
