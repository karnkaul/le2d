#include "le2d/resource/texture.hpp"
#include "klib/debug/assert.hpp"
#include "kvf/render_pass.hpp"
#include "kvf/util.hpp"

namespace le {
RenderTexture::RenderTexture(gsl::not_null<kvf::IRenderPass const*> render_pass, vk::Sampler const sampler) : sampler(sampler), m_render_pass(render_pass) {
	KLIB_ASSERT(this->sampler);
}

auto RenderTexture::get_image() const -> vk::ImageView { return m_render_pass->render_target().view; }

auto RenderTexture::get_size() const -> glm::ivec2 { return kvf::util::to_glm_vec(m_render_pass->render_target().extent); }

auto RenderTexture::descriptor_info() const -> vk::DescriptorImageInfo { return m_render_pass->render_texture_descriptor_info(sampler); }
} // namespace le
