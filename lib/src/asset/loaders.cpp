#include <le2d/asset/loaders.hpp>
#include <le2d/context.hpp>
#include <le2d/font.hpp>
#include <le2d/texture.hpp>
#include <log.hpp>

namespace le::asset {
namespace {
auto load_bytes(Context const& context, std::string_view const type, Uri const& uri) -> std::vector<std::byte> {
	auto ret = std::vector<std::byte>{};
	if (!context.get_data_loader().load_bytes(ret, uri)) {
		log::warn("'{}' Failed to load {} bytes", uri.get_string(), type);
		return {};
	}
	return ret;
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
} // namespace

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
} // namespace le::asset
