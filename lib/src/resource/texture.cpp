#include "le2d/resource/texture.hpp"
#include "kvf/render_pass.hpp"
#include "kvf/util.hpp"

namespace le {
RenderTexture::RenderTexture(gsl::not_null<kvf::RenderPass const*> render_pass, gsl::not_null<ISamplerFactory*> sampler_factory)
	: m_render_pass(render_pass), m_sampler_factory(sampler_factory) {}

auto RenderTexture::is_ready() const -> bool { return m_render_pass->render_target().view; }

auto RenderTexture::get_image() const -> vk::ImageView { return m_render_pass->render_target().view; }

auto RenderTexture::get_size() const -> glm::ivec2 { return kvf::util::to_glm_vec(m_render_pass->render_target().extent); }

auto RenderTexture::descriptor_info() const -> vk::DescriptorImageInfo {
	auto const sampler = m_sampler_factory->get_or_create(this->sampler);
	return m_render_pass->render_texture_descriptor_info(sampler);
}
} // namespace le
