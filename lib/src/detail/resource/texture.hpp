#pragma once
#include <kvf/image_bitmap.hpp>
#include <kvf/util.hpp>
#include <le2d/resource/texture.hpp>

namespace le::detail {
template <std::derived_from<ITextureBase> BaseT>
class TextureImpl : public BaseT {
  public:
	explicit TextureImpl(gsl::not_null<kvf::IRenderApi const*> render_api, TextureSampler const& sampler = {})
		: m_render_api(render_api), m_sampler(sampler), m_texture(render_api, {}, to_create_info(m_sampler)) {}

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_texture.get_image().get_view(); }
	[[nodiscard]] auto get_size() const -> glm::ivec2 final { return kvf::util::to_glm_vec<int>(m_texture.get_extent()); }

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final { return m_texture.descriptor_info(); }

	void overwrite(kvf::Bitmap const& bitmap) final { m_texture = kvf::vma::Texture{m_render_api, bitmap, to_create_info(m_sampler)}; }

	auto load_and_write(std::span<std::byte const> compressed_image) -> bool final {
		auto const image = kvf::ImageBitmap{compressed_image};
		if (!image.is_loaded()) { return false; }
		overwrite(image.bitmap());
		return true;
	}

	[[nodiscard]] auto is_ready() const -> bool final { return m_texture.is_ready(); }

  private:
	[[nodiscard]] constexpr auto to_create_info(TextureSampler const& sampler) const -> kvf::vma::TextureCreateInfo {
		auto ret = kvf::vma::TextureCreateInfo{.sampler = kvf::vma::create_sampler_ci(sampler.wrap, sampler.filter)};
		ret.sampler.setBorderColor(sampler.border);
		return ret;
	}

	gsl::not_null<kvf::IRenderApi const*> m_render_api;
	TextureSampler m_sampler;

	kvf::vma::Texture m_texture;
};

using Texture = TextureImpl<ITexture>;
using TileSheet = TextureImpl<ITileSheet>;
} // namespace le::detail
