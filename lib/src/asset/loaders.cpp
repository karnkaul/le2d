#include <le2d/asset/loaders.hpp>
#include <le2d/context.hpp>
#include <log.hpp>

namespace le::asset {
auto FontLoader::load(Uri const& uri) const -> std::unique_ptr<Wrap<Font>> {
	auto bytes = std::vector<std::byte>{};
	if (!m_context->get_data_loader().load_bytes(bytes, uri)) {
		log::warn("Failed to load font bytes: '{}'", uri.get_string());
		return {};
	}
	auto font = m_context->create_font(std::move(bytes));
	if (!font.is_loaded()) {
		log::warn("Failed to load font: '{}'", uri.get_string());
		return {};
	}

	log::debug("Font '{}' loaded from '{}'", font.get_name().as_view(), uri.get_string());
	return std::make_unique<Wrap<Font>>(std::move(font));
}
} // namespace le::asset
