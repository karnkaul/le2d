#pragma once
#include <klib/base_types.hpp>
#include <kvf/bitmap.hpp>
#include <kvf/render_device_fwd.hpp>
#include <kvf/vma.hpp>
#include <le2d/tile/tile_set.hpp>
#include <gsl/pointers>

namespace le {
class ITexture : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_image() const -> vk::ImageView = 0;
	[[nodiscard]] virtual auto get_size() const -> glm::ivec2 = 0;

	[[nodiscard]] virtual auto descriptor_info() const -> vk::DescriptorImageInfo = 0;
};

class Texture : public ITexture {
  public:
	explicit Texture(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap const& bitmap = {},
					 vk::SamplerCreateInfo const& sampler = kvf::vma::sampler_ci_v);

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_texture.get_image().get_view(); }
	[[nodiscard]] auto get_size() const -> glm::ivec2 final;

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final { return m_texture.descriptor_info(); }

	void overwrite(kvf::Bitmap const& bitmap);
	auto load_and_write(std::span<std::byte const> compressed_image) -> bool;

	[[nodiscard]] auto is_loaded() const -> bool { return m_texture.is_loaded(); }

  protected:
	gsl::not_null<kvf::RenderDevice*> m_render_device;

	kvf::vma::Texture m_texture;
};

class TileSheet : public Texture {
  public:
	explicit TileSheet(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap bitmap = {}) : Texture(render_device, bitmap) {}

	[[nodiscard]] auto get_uv(TileId const id) const -> kvf::UvRect { return tile_set.get_uv(id); }

	TileSet tile_set{};
};
} // namespace le
