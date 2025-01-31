#include <le2d/asset/loaders.hpp>
#include <le2d/context.hpp>
#include <le2d/font.hpp>
#include <le2d/texture.hpp>
#include <log.hpp>

namespace le::asset {
auto FontLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<Font>> {
	static constexpr std::string_view type_v{"Font"};
	auto bytes = std::vector<std::byte>{};
	if (!m_context->get_data_loader().load_bytes(bytes, uri)) {
		log::warn("Failed to load {} bytes: '{}'", type_v, uri.get_string());
		return {};
	}
	if (bytes.empty()) { return {}; }
	auto font = m_context->create_font(std::move(bytes));
	if (!font.is_loaded()) {
		log::warn("Failed to load {}: '{}'", type_v, uri.get_string());
		return {};
	}

	log::info("=='{}'== {} '{}' loaded", uri.get_string(), type_v, font.get_name().as_view());
	return std::make_unique<Wrap<Font>>(std::move(font));
}

auto TextureLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<Texture>> {
	static constexpr std::string_view type_v{"Texture"};
	auto bytes = std::vector<std::byte>{};
	if (!m_context->get_data_loader().load_bytes(bytes, uri)) {
		log::warn("Failed to load {} bytes: '{}'", type_v, uri.get_string());
		return {};
	}
	if (bytes.empty()) { return {}; }
	auto texture = m_context->create_texture();
	if (!texture.load_and_write(bytes)) {
		log::warn("Failed to load {}: '{}'", type_v, uri.get_string());
		return {};
	}

	log::info("=='{}'== {} loaded", type_v, uri.get_string());
	return std::make_unique<Wrap<Texture>>(std::move(texture));
}
} // namespace le::asset
