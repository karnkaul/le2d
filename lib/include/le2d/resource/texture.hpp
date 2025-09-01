#pragma once
#include <kvf/bitmap.hpp>
#include <kvf/render_device_fwd.hpp>
#include <kvf/vma.hpp>
#include <le2d/resource/resource.hpp>
#include <le2d/tile/tile_set.hpp>
#include <gsl/pointers>

namespace kvf {
class RenderPass;
} // namespace kvf

namespace le {
/// \brief Interface for drawable texture.
class ITextureBase : public IResource {
  public:
	[[nodiscard]] virtual auto get_image() const -> vk::ImageView = 0;
	[[nodiscard]] virtual auto get_size() const -> glm::ivec2 = 0;

	[[nodiscard]] virtual auto descriptor_info() const -> vk::DescriptorImageInfo = 0;
};

/// \brief Texture Sampler metadata.
struct TextureSampler {
	vk::SamplerAddressMode wrap{vk::SamplerAddressMode::eClampToEdge};
	vk::Filter filter{vk::Filter::eLinear};
	vk::BorderColor border{vk::BorderColor::eIntOpaqueBlack};
};

/// \brief Concrete drawable Texture.
class ITexture : public ITextureBase {
  public:
	/// \brief Write bitmap to image.
	/// \param bitmap Bitmap to write.
	virtual void overwrite(kvf::Bitmap const& bitmap) = 0;
	/// \brief Load a compressed bitmap and write to image.
	/// \param compressed_image Bytes of compressed image.
	/// \returns true if successfully decompressed.
	virtual auto load_and_write(std::span<std::byte const> compressed_image) -> bool = 0;
};

/// \brief Texture with a TileSet.
class ITileSheet : public ITexture {
  public:
	/// \brief Get the UV coordinates for a given Tile ID.
	/// \param id Tile ID to query.
	/// \returns UV rect for tile if found, else uv_rect_v.
	[[nodiscard]] auto get_uv(TileId const id) const -> kvf::UvRect { return tile_set.get_uv(id); }

	TileSet tile_set{};
};

/// \brief Wraps a RenderTarget as a texture.
/// This is just a view type, it doesn't own any resources.
class RenderTexture : public ITextureBase {
  public:
	/// \param render_pass RenderTarget source. Must outlive RenderTexture.
	explicit RenderTexture(gsl::not_null<kvf::RenderPass const*> render_pass);

	[[nodiscard]] auto is_ready() const -> bool final;

	[[nodiscard]] auto get_image() const -> vk::ImageView final;
	[[nodiscard]] auto get_size() const -> glm::ivec2 final;

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final;

  private:
	gsl::not_null<kvf::RenderPass const*> m_render_pass;
};
} // namespace le
