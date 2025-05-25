#include <le2d/asset/loaders.hpp>
#include <le2d/context.hpp>
#include <le2d/json_io.hpp>
#include <log.hpp>

namespace le::asset {
namespace {
auto load_bytes(Context const& context, std::string_view const type, Uri const& uri) -> std::vector<std::byte> {
	auto ret = std::vector<std::byte>{};
	if (!context.get_data_loader().load_bytes(ret, uri.get_string())) {
		log.warn("'{}' Failed to load {} bytes", uri.get_string(), type);
		return {};
	}
	return ret;
}

auto load_json(Context const& context, std::string_view const type, Uri const& uri) -> dj::Json {
	auto text = std::string{};
	if (!context.get_data_loader().load_string(text, uri.get_string())) {
		log.warn("'{}' Failed to load {} JSON", uri.get_string(), type);
		return {};
	}
	return dj::Json::parse(text);
}

template <typename T, typename F, typename... Args>
auto try_load(Uri const& uri, std::string_view const type, T& t, F func, Args&&... args) {
	if (!std::invoke(func, &t, std::forward<Args>(args)...)) {
		log.warn("'{}' Failed to load {}", uri.get_string(), type);
		return false;
	}
	return true;
}

template <typename T>
auto to_wrap(Uri const& uri, std::string_view const type, T t) {
	log.info("=='{}'== {} loaded", uri.get_string(), type);
	return std::make_unique<Wrap<T>>(std::move(t));
}
} // namespace

auto JsonLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<dj::Json>> {
	static constexpr std::string_view type_v{"JSON"};
	auto json = dj::Json{};
	if (!m_context->get_data_loader().load_json(json, uri.get_string())) {
		log.warn("'{}' Failed to load {} bytes", uri.get_string(), type_v);
		return {};
	}
	return to_wrap(uri, type_v, std::move(json));
}

auto SpirVLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<SpirV>> {
	static constexpr std::string_view type_v{"SpirV"};
	auto ret = std::vector<std::uint32_t>{};
	if (!m_context->get_data_loader().load_spirv(ret, uri.get_string())) {
		log.warn("'{}' Failed to load {} bytes", uri.get_string(), type_v);
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

auto TileSetLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<TileSet>> {
	static constexpr std::string_view type_v{"TileSet"};
	auto const json = load_json(*m_context, type_v, uri);
	auto tile_set = TileSet{};
	if (!is_json_type(json, tile_set)) { return {}; }
	from_json(json, tile_set);
	return to_wrap(uri, type_v, std::move(tile_set));
}

auto TextureLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<Texture>> {
	static constexpr std::string_view type_v{"Texture"};
	auto const bytes = load_bytes(*m_context, type_v, uri);
	if (bytes.empty()) { return {}; }
	auto texture = m_context->create_texture();
	if (!try_load(uri, type_v, texture, &Texture::load_and_write, bytes)) { return {}; }
	return to_wrap(uri, type_v, std::move(texture));
}

auto TileSheetLoader::load(Uri const& uri, Uri& out_texture_uri) const -> std::unique_ptr<Wrap<TileSheet>> {
	static constexpr std::string_view type_v{"TileSheet"};
	auto const json = load_json(*m_context, type_v, uri);
	if (!is_json_type<TileSheet>(json)) { return {}; }

	out_texture_uri = json["texture"].as<std::string>();
	auto const bytes = load_bytes(*m_context, type_v, out_texture_uri);
	if (bytes.empty()) { return {}; }

	auto tile_sheet = m_context->create_tilesheet();
	if (!try_load(uri, type_v, tile_sheet, &TileSheet::load_and_write, bytes)) { return {}; }

	auto const& tile_set_json = json["tile_set"];
	if (tile_set_json.is_string()) {
		auto const tile_set_loader = TileSetLoader{m_context};
		auto tile_set = tile_set_loader.load(json["tile_set"].as<std::string>());
		if (tile_set) { tile_sheet.tile_set = std::move(tile_set->asset); }
	} else {
		from_json(tile_set_json, tile_sheet.tile_set);
	}

	return to_wrap(uri, type_v, std::move(tile_sheet));
}

auto TileSheetLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<TileSheet>> {
	auto texture_uri = Uri{};
	return load(uri, texture_uri);
}

auto TransformAnimationLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<anim::Animation<Transform>>> {
	static constexpr std::string_view type_v{"TransformAnimation"};
	auto const json = load_json(*m_context, type_v, uri);
	auto animation = anim::Animation<Transform>{};
	if (!is_json_type(json, animation)) { return {}; }
	from_json(json, animation);
	return to_wrap(uri, type_v, std::move(animation));
}

auto TileAnimationLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<anim::Animation<TileId>>> {
	static constexpr std::string_view type_v{"TileAnimation"};
	auto const json = load_json(*m_context, type_v, uri);
	auto animation = anim::Animation<TileId>{};
	if (!is_json_type(json, animation)) { return {}; }
	from_json(json, animation);
	return to_wrap(uri, type_v, std::move(animation));
}

auto AudioBufferLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<capo::Buffer>> {
	static constexpr std::string_view type_v{"AudioBuffer"};
	auto const bytes = load_bytes(*m_context, type_v, uri);
	if (bytes.empty()) { return {}; }
	auto const encoding = capo::guess_encoding(uri.get_string().data());
	auto buffer = capo::Buffer{};
	if (!buffer.decode_bytes(bytes, encoding)) {
		log.warn("'{}' Failed to load {}", uri.get_string(), type_v);
		return {};
	}
	return to_wrap(uri, type_v, std::move(buffer));
}
} // namespace le::asset
