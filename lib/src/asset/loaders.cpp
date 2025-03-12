#include <le2d/asset/loaders.hpp>
#include <le2d/context.hpp>
#include <le2d/json_io.hpp>
#include <log.hpp>
#include <filesystem>

namespace le::asset {
namespace fs = std::filesystem;

namespace {
auto load_bytes(Context const& context, std::string_view const type, Uri const& uri) -> std::vector<std::byte> {
	auto ret = std::vector<std::byte>{};
	if (!context.get_data_loader().load_bytes(ret, uri)) {
		log::warn("'{}' Failed to load {} bytes", uri.get_string(), type);
		return {};
	}
	return ret;
}

auto load_json(Context const& context, std::string_view const type, Uri const& uri) -> dj::Json {
	auto text = std::string{};
	if (!context.get_data_loader().load_string(text, uri)) {
		log::warn("'{}' Failed to load {} JSON", uri.get_string(), type);
		return {};
	}
	return dj::Json::parse(text);
}

template <typename T, typename F, typename... Args>
auto try_load(Uri const& uri, std::string_view const type, T& t, F func, Args&&... args) {
	if (!std::invoke(func, &t, std::forward<Args>(args)...)) {
		log::warn("'{}' Failed to load {}", uri.get_string(), type);
		return false;
	}
	return true;
}

template <typename T>
auto to_wrap(Uri const& uri, std::string_view const type, T t) {
	log::info("=='{}'== {} loaded", uri.get_string(), type);
	return std::make_unique<Wrap<T>>(std::move(t));
}

constexpr auto to_compression(std::string_view const extension) {
	if (extension == ".wav") { return capo::Compression::eWav; }
	if (extension == ".mp3") { return capo::Compression::eMp3; }
	if (extension == ".flac") { return capo::Compression::eFlac; }
	return capo::Compression::eUnknown;
}
} // namespace

auto JsonLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<dj::Json>> {
	static constexpr std::string_view type_v{"JSON"};
	auto json = dj::Json{};
	if (!m_context->get_data_loader().load_json(json, uri)) {
		log::warn("'{}' Failed to load {} bytes", uri.get_string(), type_v);
		return {};
	}
	return to_wrap(uri, type_v, std::move(json));
}

auto SpirVLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<SpirV>> {
	static constexpr std::string_view type_v{"SpirV"};
	auto ret = std::vector<std::uint32_t>{};
	if (!m_context->get_data_loader().load_spirv(ret, uri)) {
		log::warn("'{}' Failed to load {} bytes", uri.get_string(), type_v);
		return {};
	}
	return to_wrap(uri, type_v, SpirV{.code = std::move(ret)});
}

auto FontLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<Font>> {
	static constexpr std::string_view type_v{"Font"};
	auto bytes = load_bytes(*m_context, type_v, uri);
	if (bytes.empty()) { return {}; }
	auto font = m_context->create_font();
	if (!try_load(uri, type_v, font, &Font::load_face, std::move(bytes))) { return {}; }
	return to_wrap(uri, type_v, std::move(font));
}

auto TextureLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<Texture>> {
	static constexpr std::string_view type_v{"Texture"};
	auto const bytes = load_bytes(*m_context, type_v, uri);
	if (bytes.empty()) { return {}; }
	auto texture = m_context->create_texture();
	if (!try_load(uri, type_v, texture, &Texture::load_and_write, bytes)) { return {}; }
	return to_wrap(uri, type_v, std::move(texture));
}

auto TileSheetLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<TileSheet>> {
	static constexpr std::string_view type_v{"TileSheet"};
	auto const json = load_json(*m_context, type_v, uri);
	if (!json) { return {}; }
	auto const texture_uri = Uri{json["texture"].as<std::string>()};
	auto const bytes = load_bytes(*m_context, type_v, texture_uri);
	if (bytes.empty()) { return {}; }
	auto tile_set = m_context->create_tilesheet();
	if (!try_load(uri, type_v, tile_set, &TileSheet::load_and_write, bytes)) { return {}; }
	auto tiles = std::vector<Tile>{};
	auto const& in_tiles = json["tiles"].array_view();
	tiles.reserve(in_tiles.size());
	for (auto const& in_tile : in_tiles) { from_json(in_tile, tiles.emplace_back()); }
	tile_set.set_tiles(std::move(tiles));
	return to_wrap(uri, type_v, std::move(tile_set));
}

auto TransformAnimationLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<anim::Animation<Transform>>> {
	static constexpr std::string_view type_v{"TransformAnimation"};
	auto const json = load_json(*m_context, type_v, uri);
	if (!json) { return {}; }
	auto animation = anim::Animation<Transform>{};
	from_json(json, animation);
	return to_wrap(uri, type_v, std::move(animation));
}

auto TileAnimationLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<anim::Animation<TileId>>> {
	static constexpr std::string_view type_v{"TileAnimation"};
	auto const json = load_json(*m_context, type_v, uri);
	if (!json) { return {}; }
	auto animation = anim::Animation<TileId>{};
	from_json(json, animation);
	return to_wrap(uri, type_v, std::move(animation));
}

auto PcmLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<capo::Pcm>> {
	static constexpr std::string_view type_v{"Pcm"};
	auto const bytes = load_bytes(*m_context, type_v, uri);
	if (bytes.empty()) { return {}; }
	auto const extension = fs::path{uri.get_string()}.extension().string();
	auto const compression = to_compression(extension);
	auto result = capo::Pcm::make(bytes, compression);
	if (!result) {
		log::warn("'{}' Failed to load {}", uri.get_string(), type_v);
		return {};
	}
	return to_wrap(uri, type_v, std::move(result.pcm));
}
} // namespace le::asset
