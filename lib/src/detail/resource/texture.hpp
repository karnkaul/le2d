#pragma once
#include "kvf/image_bitmap.hpp"
#include "kvf/render_api.hpp"
#include "kvf/util.hpp"
#include "kvf/vma.hpp"
#include "le2d/resource/sampler_factory.hpp"
#include "le2d/resource/texture.hpp"

namespace le::detail {
template <std::derived_from<ITextureBase> BaseT>
class TextureImpl : public BaseT {
  public:
	explicit TextureImpl(gsl::not_null<kvf::IRenderApi const*> render_api, gsl::not_null<ISamplerFactory*> sampler_factory, TextureSampler const& sampler = {})
		: m_render_api(render_api), m_sampler_factory(sampler_factory), m_texture(render_api) {
		this->sampler = sampler;
	}

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_texture.get_image().get_view(); }
	[[nodiscard]] auto get_size() const -> glm::ivec2 final { return kvf::util::to_glm_vec<int>(m_texture.get_extent()); }

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final {
		auto const sampler = m_sampler_factory->get_or_create(this->sampler);
		return m_texture.descriptor_info(sampler);
	}

	void overwrite(kvf::Bitmap const& bitmap) final { m_texture = kvf::vma::Texture{m_render_api, bitmap}; }

	auto load_and_write(std::span<std::byte const> compressed_image) -> bool final {
		auto const image = kvf::ImageBitmap{compressed_image};
		if (!image.is_loaded()) { return false; }
		overwrite(image.bitmap());
		return true;
	}

	[[nodiscard]] auto is_ready() const -> bool final { return m_texture.is_ready(); }

  private:
	gsl::not_null<kvf::IRenderApi const*> m_render_api;
	gsl::not_null<ISamplerFactory*> m_sampler_factory;

	kvf::vma::Texture m_texture;
};

using Texture = TextureImpl<ITexture>;
using TileSheet = TextureImpl<ITileSheet>;
} // namespace le::detail
