#pragma once
#include "kvf/image.hpp"
#include "kvf/image_bitmap.hpp"
#include "kvf/render_device.hpp"
#include "kvf/util.hpp"
#include "le2d/resource/sampler_factory.hpp"
#include "le2d/resource/texture.hpp"

namespace le::detail {
template <std::derived_from<ITextureBase> BaseT>
class TextureImpl : public BaseT {
  public:
	explicit TextureImpl(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory, TextureSampler const& sampler = {})
		: m_render_device(render_device), m_sampler_factory(sampler_factory), m_texture(kvf::IImage::create_texture(render_device, {})) {
		this->sampler = sampler;
	}

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_texture->get_image_view(); }

	[[nodiscard]] auto get_size() const -> glm::ivec2 final { return kvf::util::to_glm_vec<int>(m_texture->get_extent()); }

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final {
		if (!m_cached_sampler.vk_sampler || m_cached_sampler.sampler != this->sampler) {
			m_cached_sampler.sampler = this->sampler;
			m_cached_sampler.vk_sampler = m_sampler_factory->get_or_create(this->sampler);
		}
		return m_texture->descriptor_info(m_cached_sampler.vk_sampler);
	}

	void overwrite(kvf::Bitmap const& bitmap) final { m_texture = kvf::IImage::create_texture(m_render_device, bitmap); }

	auto load_and_write(std::span<std::byte const> compressed_image) -> bool final {
		auto const image = kvf::ImageBitmap{compressed_image};
		if (!image.is_loaded()) { return false; }
		overwrite(image.bitmap());
		return true;
	}

	[[nodiscard]] auto is_ready() const -> bool final { return m_texture != nullptr; }

  private:
	struct CachedSampler {
		TextureSampler sampler{};
		vk::Sampler vk_sampler{};
	};

	gsl::not_null<kvf::IRenderDevice*> m_render_device;
	gsl::not_null<ISamplerFactory*> m_sampler_factory;

	std::unique_ptr<kvf::IImage> m_texture;
	mutable CachedSampler m_cached_sampler{};
};

using Texture = TextureImpl<ITexture>;
using TileSheet = TextureImpl<ITileSheet>;
} // namespace le::detail
