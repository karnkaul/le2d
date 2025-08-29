#pragma once
#include <kvf/device_waiter.hpp>
#include <kvf/render_device.hpp>
#include <kvf/render_pass.hpp>
#include <kvf/util.hpp>
#include <le2d/render_pass.hpp>
#include <algorithm>

namespace le::detail {
class RenderPass : public IRenderPass {
  public:
	static constexpr auto color_format_v{vk::Format::eR8G8B8A8Srgb};

	static constexpr auto clamp_size(glm::ivec2 in) {
		in.x = std::clamp(in.x, RenderPass::min_size_v, RenderPass::max_size_v);
		in.y = std::clamp(in.y, RenderPass::min_size_v, RenderPass::max_size_v);
		return in;
	}

	explicit RenderPass(gsl::not_null<kvf::RenderDevice*> render_device, gsl::not_null<IResourcePool*> resource_pool, vk::SampleCountFlagBits samples)
		: m_render_device(render_device), m_resource_pool(resource_pool), m_render_pass(render_device, samples), m_waiter(render_device->get_device()) {
		m_render_pass.set_color_target(color_format_v);
	}

	[[nodiscard]] auto get_render_device() const -> kvf::RenderDevice& final { return *m_render_device; }

	[[nodiscard]] auto get_render_target() const -> kvf::RenderTarget const& final { return m_render_pass.render_target(); }

	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits final { return m_render_pass.get_samples(); }

	[[nodiscard]] auto render_texture() const -> RenderTexture final { return RenderTexture{&m_render_pass}; }

	void set_clear_color(kvf::Color const color) final { m_render_pass.clear_color = color.to_linear(); }

	auto begin_render(vk::CommandBuffer command_buffer, glm::ivec2 size) -> Renderer final {
		size = clamp_size(size);
		m_render_pass.begin_render(command_buffer, kvf::util::to_vk_extent(size));
		return Renderer{m_render_pass, *m_resource_pool};
	}

  private:
	gsl::not_null<kvf::RenderDevice*> m_render_device;
	gsl::not_null<IResourcePool*> m_resource_pool;
	kvf::RenderPass m_render_pass;

	kvf::DeviceWaiter m_waiter;
};
} // namespace le::detail
