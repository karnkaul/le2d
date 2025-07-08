#include <klib/assert.hpp>
#include <kvf/render_device.hpp>
#include <kvf/util.hpp>
#include <le2d/render_pass.hpp>

namespace le {
namespace {
constexpr auto color_format_v{vk::Format::eR8G8B8A8Srgb};

constexpr auto clamp_size(glm::ivec2 in) {
	in.x = std::clamp(in.x, RenderPass::min_size_v, RenderPass::max_size_v);
	in.y = std::clamp(in.y, RenderPass::min_size_v, RenderPass::max_size_v);
	return in;
}
} // namespace

RenderPass::RenderPass(gsl::not_null<kvf::RenderDevice*> render_device, vk::SampleCountFlagBits samples)
	: m_render_device(render_device), m_render_pass(std::make_unique<kvf::RenderPass>(render_device, samples)), m_waiter(render_device->get_device()) {
	m_render_pass->set_color_target(color_format_v);
}

auto RenderPass::get_render_target() const -> kvf::RenderTarget const& {
	KLIB_ASSERT(is_ready());
	return m_render_pass->render_target();
}

void RenderPass::set_clear_color(kvf::Color const color) {
	KLIB_ASSERT(is_ready());
	m_render_pass->clear_color = color.to_linear();
}

auto RenderPass::get_samples() const -> vk::SampleCountFlagBits {
	KLIB_ASSERT(is_ready());
	return m_render_pass->get_samples();
}

auto RenderPass::begin_render(IResourcePool& resource_pool, vk::CommandBuffer command_buffer, glm::ivec2 size) -> Renderer {
	KLIB_ASSERT(is_ready());
	size = clamp_size(size);
	m_render_pass->begin_render(command_buffer, kvf::util::to_vk_extent(size));
	return Renderer{*m_render_pass, resource_pool};
}

auto RenderPass::render_texture() const -> RenderTexture { return RenderTexture{m_render_pass.get()}; }
} // namespace le
