#include <kvf/color.hpp>
#include <kvf/image_bitmap.hpp>
#include <kvf/render_device.hpp>
#include <kvf/util.hpp>
#include <le2d/texture.hpp>

namespace le {
Texture::Texture(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap bitmap, vk::SamplerCreateInfo const& sampler)
	: m_render_device(render_device), m_texture(render_device, bitmap, kvf::vma::TextureCreateInfo{.sampler = sampler}) {}

auto Texture::get_size() const -> glm::ivec2 { return kvf::util::to_glm_vec<int>(m_texture.get_extent()); }

void Texture::overwrite(kvf::Bitmap const& bitmap) { m_texture = kvf::vma::Texture{m_render_device.get(), bitmap}; }

auto Texture::load_and_write(std::span<std::byte const> compressed_image) -> bool {
	auto const bitmap = kvf::ImageBitmap{compressed_image};
	if (!bitmap.is_loaded()) { return false; }
	overwrite(bitmap.bitmap());
	return true;
}
} // namespace le
