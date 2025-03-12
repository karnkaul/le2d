#pragma once
#include <klib/base_types.hpp>
#include <kvf/bitmap.hpp>
#include <kvf/render_device_fwd.hpp>
#include <kvf/vma.hpp>
#include <le2d/texture_sampler.hpp>
#include <le2d/tile/tile_set.hpp>
#include <gsl/pointers>

namespace le {
class ITexture : public klib::Polymorphic {
  public:
	using Sampler = TextureSampler;

	[[nodiscard]] virtual auto get_image() const -> vk::ImageView = 0;
	[[nodiscard]] virtual auto get_size() const -> glm::ivec2 = 0;

	Sampler sampler{};
};

class Texture : public ITexture {
  public:
	explicit Texture(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap bitmap = {});

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_image.get_view(); }
	[[nodiscard]] auto get_size() const -> glm::ivec2 final;

	auto write(kvf::Bitmap const& bitmap) -> bool;
	auto load_and_write(std::span<std::byte const> compressed_image) -> bool;

  protected:
	gsl::not_null<kvf::RenderDevice*> m_render_device;

	kvf::vma::Image m_image{};
};

class TileSheet : public Texture {
  public:
	explicit TileSheet(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap bitmap = {}) : Texture(render_device, bitmap) {}

	[[nodiscard]] auto get_uv(TileId const id) const -> kvf::UvRect { return tile_set.get_uv(id); }

	TileSet tile_set{};
};
} // namespace le
