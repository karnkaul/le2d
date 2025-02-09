#include <le2d/asset/loaders.hpp>
#include <le2d/context.hpp>
#include <log.hpp>

namespace le {
namespace {
auto create_window(WindowCreateInfo const& create_info) {
	struct Visitor {
		auto operator()(WindowInfo const& info) const { return RenderWindow{info.size, info.title, info.decorated}; }
		auto operator()(FullscreenInfo const& info) const { return RenderWindow{info.title, info.target}; }
	};
	return std::visit(Visitor{}, create_info);
}
} // namespace

Context::Context(gsl::not_null<IDataLoader const*> data_loader, CreateInfo const& create_info)
	: m_data_loader(data_loader), m_window(create_window(create_info.window)), m_resource_pool(&m_window.get_render_device()),
	  m_pass(&m_window.get_render_device(), create_info.framebuffer_samples) {
	auto const& shader = create_info.default_shader;
	m_resource_pool.default_shader = create_shader(shader.vertex, shader.fragment);
	if (!m_resource_pool.default_shader) {
		log::warn("Context: failed to create Default Shader: '{}' / '{}'", shader.vertex.get_string(), shader.fragment.get_string());
	}
}

auto Context::framebuffer_size() const -> glm::ivec2 { return glm::vec2{swapchain_size()} * m_render_scale; }

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

auto Context::begin_render(kvf::Color const clear) -> Renderer {
	if (!m_cmd) { return {}; }
	m_pass.set_clear_color(clear);
	return m_pass.begin_render(m_resource_pool, m_cmd, framebuffer_size());
}

void Context::present() {
	m_window.present(m_pass.get_render_target());
	m_cmd = vk::CommandBuffer{};
}

auto Context::create_shader(Uri const& vertex, Uri const& fragment) const -> Shader {
	auto const loader = asset::SpirVLoader{this};
	auto const vert = loader.load(vertex);
	auto const frag = loader.load(fragment);
	if (!vert || !frag) { return {}; }
	return Shader{m_pass.get_render_device().get_device(), vert->asset, frag->asset};
}

auto Context::create_render_pass(vk::SampleCountFlagBits const samples) const -> RenderPass { return RenderPass{&m_pass.get_render_device(), samples}; }

auto Context::create_texture(kvf::Bitmap const& bitmap) const -> Texture { return Texture{&m_pass.get_render_device(), bitmap}; }

auto Context::create_font(std::vector<std::byte> font_bytes) const -> Font { return Font{&m_pass.get_render_device(), std::move(font_bytes)}; }

auto Context::create_asset_load_task(gsl::not_null<klib::task::Queue*> task_queue) const -> std::unique_ptr<asset::LoadTask> {
	auto ret = std::make_unique<asset::LoadTask>(task_queue);
	ret->add_loader(std::make_unique<asset::JsonLoader>(this));
	ret->add_loader(std::make_unique<asset::SpirVLoader>(this));
	ret->add_loader(std::make_unique<asset::FontLoader>(this));
	ret->add_loader(std::make_unique<asset::TextureLoader>(this));
	ret->add_loader(std::make_unique<asset::AnimationLoader>(this));
	ret->add_loader(std::make_unique<asset::FlipbookLoader>(this));
	return ret;
}
} // namespace le
