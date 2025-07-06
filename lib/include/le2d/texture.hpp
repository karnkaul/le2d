#pragma once
#include <klib/base_types.hpp>
#include <kvf/bitmap.hpp>
#include <kvf/render_device_fwd.hpp>
#include <kvf/vma.hpp>
#include <le2d/tile/tile_set.hpp>
#include <gsl/pointers>

namespace le {
/// \brief Interface for drawable texture.
class ITexture : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_image() const -> vk::ImageView = 0;
	[[nodiscard]] virtual auto get_size() const -> glm::ivec2 = 0;

	[[nodiscard]] virtual auto descriptor_info() const -> vk::DescriptorImageInfo = 0;

	[[nodiscard]] virtual auto is_ready() const -> bool = 0;
};

/// \brief Concrete drawable Texture.
class Texture : public ITexture {
  public:
	explicit Texture(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap const& bitmap = {},
					 vk::SamplerCreateInfo const& sampler = kvf::vma::sampler_ci_v);

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_texture.get_image().get_view(); }
	[[nodiscard]] auto get_size() const -> glm::ivec2 final;

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final { return m_texture.descriptor_info(); }

	/// \brief Write bitmap to image.
	/// \param bitmap Bitmap to write.
	void overwrite(kvf::Bitmap const& bitmap);
	/// \brief Load a compressed bitmap and write to image.
	/// \param compressed_image Bytes of compressed image.
	/// \returns true if successfully decompressed.
	auto load_and_write(std::span<std::byte const> compressed_image) -> bool;

	/// \returns true if a valid Image has been allocated and can be bound to descriptor sets.
	[[nodiscard]] auto is_ready() const -> bool final { return m_texture.is_ready(); }

  protected:
	gsl::not_null<kvf::RenderDevice*> m_render_device;

	kvf::vma::Texture m_texture;
};

/// \brief Texture with a TileSet.
class TileSheet : public Texture {
  public:
	explicit TileSheet(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap bitmap = {}) : Texture(render_device, bitmap) {}

	/// \brief Get the UV coordinates for a given Tile ID.
	/// \param id Tile ID to query.
	/// \returns UV rect for tile if found, else uv_rect_v.
	[[nodiscard]] auto get_uv(TileId const id) const -> kvf::UvRect { return tile_set.get_uv(id); }

	TileSet tile_set{};
};
} // namespace le
