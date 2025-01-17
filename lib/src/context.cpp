#include <le2d/context.hpp>
#include <log.hpp>

namespace le {
Context::Context(gsl::not_null<IDataLoader const*> data_loader, CreateInfo const& create_info)
	: m_data_loader(data_loader), m_window(create_info.window_size, create_info.window_title), m_resource_pool(&m_window.get_render_device()),
	  m_pass(&m_window.get_render_device(), &m_resource_pool, create_info.framebuffer_samples) {
	auto const& shader = create_info.default_shader;
	m_resource_pool.default_shader = create_shader(shader.vertex, shader.fragment);
	if (!m_resource_pool.default_shader) {
		log::warn("Context: failed to create Default Shader: '{}' / '{}'", shader.vertex.get_string(), shader.fragment.get_string());
	}
}

auto Context::set_render_scale(float const scale) -> bool {
	if (scale < min_render_scale_v || scale > max_render_scale_v) { return false; }
	m_render_scale = scale;
	return true;
}

auto Context::next_frame() -> vk::CommandBuffer {
	m_cmd = m_window.next_frame();
	m_resource_pool.next_frame();
	return m_cmd;
}

auto Context::begin_render() -> Renderer {
	if (!m_cmd) { return {}; }
	glm::ivec2 const scaled_extent = glm::vec2{swapchain_size()} * m_render_scale;
	return m_pass.begin_render(m_cmd, scaled_extent);
}

void Context::present() {
	m_window.present(m_pass.get_render_target());
	m_cmd = vk::CommandBuffer{};
}

auto Context::create_shader(Uri const& vertex, Uri const& fragment) const -> Shader {
	return Shader{*m_data_loader, m_pass.get_render_device().get_device(), vertex, fragment};
}

auto Context::create_render_pass(vk::SampleCountFlagBits const samples) const -> RenderPass {
	return RenderPass{&m_pass.get_render_device(), &m_resource_pool, samples};
}

auto Context::create_texture(kvf::Bitmap const& bitmap) const -> Texture { return Texture{&m_pass.get_render_device(), bitmap}; }

auto Context::create_font(std::vector<std::byte> font_bytes) const -> Font { return Font{&m_pass.get_render_device(), std::move(font_bytes)}; }
} // namespace le
