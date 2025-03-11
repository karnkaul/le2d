#include <common.hpp>
#include <kvf/color.hpp>
#include <kvf/image_bitmap.hpp>
#include <kvf/util.hpp>
#include <le2d/texture.hpp>
#include <algorithm>

namespace le {
namespace {
constexpr auto texture_ici() {
	return kvf::vma::ImageCreateInfo{
		.format = color_format_v,
		.aspect = vk::ImageAspectFlagBits::eColor,
		.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		.flags = kvf::vma::ImageFlag::MipMapped,
	};
}
} // namespace

Texture::Texture(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap bitmap)
	: m_render_device(render_device), m_image(render_device, texture_ici(), kvf::util::to_vk_extent(bitmap.size)) {
	if (bitmap.bytes.empty() || bitmap.size.x <= 0 || bitmap.size.y <= 0) { bitmap = white_bitmap_v; }
	write(bitmap);
}

auto Texture::get_size() const -> glm::ivec2 { return kvf::util::to_glm_vec<int>(m_image.get_extent()); }

auto Texture::write(kvf::Bitmap const& bitmap) -> bool { return kvf::util::write_to(m_image, bitmap); }

auto Texture::load_and_write(std::span<std::byte const> compressed_image) -> bool {
	if (!m_image) { return false; }
	auto const bitmap = kvf::ImageBitmap{compressed_image};
	if (!bitmap.is_loaded()) { return false; }
	return kvf::util::write_to(m_image, bitmap.bitmap());
}

void TileSet::set_tiles(std::vector<Tile> tiles) {
	m_tiles = std::move(tiles);
	std::ranges::sort(m_tiles, [](Tile const& a, Tile const& b) { return a.id < b.id; });
}

auto TileSet::get_uv(TileId const id) const -> kvf::UvRect {
	if (id == TileId::None) { return kvf::uv_rect_v; }
	auto const [first, _] = std::ranges::equal_range(m_tiles, id, {}, [](Tile const& t) { return t.id; });
	if (first == m_tiles.end()) { return kvf::uv_rect_v; }
	return first->uv;
}
} // namespace le
