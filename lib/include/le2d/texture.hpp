#pragma once
#include <klib/polymorphic.hpp>
#include <kvf/bitmap.hpp>
#include <kvf/render_device_fwd.hpp>
#include <kvf/vma.hpp>
#include <le2d/texture_sampler.hpp>
#include <gsl/pointers>

namespace le {
class ITexture : public klib::Polymorphic {
  public:
	using Sampler = TextureSampler;

	[[nodiscard]] virtual auto get_image() const -> vk::ImageView = 0;

	Sampler sampler{};
};

class Texture : public ITexture {
  public:
	explicit Texture(gsl::not_null<kvf::RenderDevice*> render_device, kvf::Bitmap bitmap = {});

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_image.get_view(); }

	auto write(kvf::Bitmap const& bitmap) -> bool;
	auto load_and_write(std::span<std::byte const> compressed_image) -> bool;

  protected:
	gsl::not_null<kvf::RenderDevice*> m_render_device;

	kvf::vma::Image m_image{};
};
} // namespace le
