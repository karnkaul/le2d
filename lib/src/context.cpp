#include <detail/pipeline_pool.hpp>
#include <klib/assert.hpp>
#include <le2d/asset/loaders.hpp>
#include <le2d/context.hpp>
#include <log.hpp>

namespace le {
namespace {
constexpr auto to_vsync(vk::PresentModeKHR const mode) {
	switch (mode) {
	case vk::PresentModeKHR::eFifoRelaxed: return Vsync::Adaptive;
	case vk::PresentModeKHR::eMailbox: return Vsync::MultiBuffer;
	case vk::PresentModeKHR::eImmediate: return Vsync::Off;
	default:
	case vk::PresentModeKHR::eFifo: return Vsync::Strict;
	}
}

constexpr auto to_mode(Vsync const vsync) {
	switch (vsync) {
	case Vsync::Adaptive: return vk::PresentModeKHR::eFifoRelaxed;
	case Vsync::MultiBuffer: return vk::PresentModeKHR::eMailbox;
	case Vsync::Off: return vk::PresentModeKHR::eImmediate;
	default:
	case Vsync::Strict: return vk::PresentModeKHR::eFifo;
	}
}

struct ResourcePool : IResourcePool {
	explicit ResourcePool(gsl::not_null<kvf::RenderDevice*> render_device, ShaderProgram default_shader)
		: m_pipelines(render_device), m_default_shader(std::move(default_shader)), m_white_texture(render_device), m_blocker(render_device->get_device()) {}

	[[nodiscard]] auto allocate_pipeline(PipelineFixedState const& state, ShaderProgram const& shader) -> vk::Pipeline final {
		return m_pipelines.allocate(state, shader);
	}

	[[nodiscard]] auto get_pipeline_layout() const -> vk::PipelineLayout final { return m_pipelines.get_layout(); }
	[[nodiscard]] auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const> final { return m_pipelines.get_set_layouts(); }
	[[nodiscard]] auto get_white_texture() const -> Texture const& final { return m_white_texture; }
	[[nodiscard]] auto get_default_shader() const -> ShaderProgram const& final { return m_default_shader; }

  private:
	detail::PipelinePool m_pipelines;
	ShaderProgram m_default_shader{};

	Texture m_white_texture;

	kvf::DeviceBlock m_blocker;
};

struct Audio : IAudio {
	explicit Audio(int sfx_sources) {
		if (!m_device) {
			// TODO: log error
		}
		static constexpr auto max_sfx_sources_v{256};
		sfx_sources = std::clamp(sfx_sources, 1, max_sfx_sources_v);
		m_sfx_sources.reserve(std::size_t(sfx_sources));
		for (int i = 0; i < sfx_sources; ++i) {
			auto& sfx_source = m_sfx_sources.emplace_back();
			sfx_source.source = m_device.make_sound_source();
		}
	}

	[[nodiscard]] auto get_sfx_gain() const -> float final { return m_sfx_gain; }

	void set_sfx_gain(float const gain) final {
		if (std::abs(gain - m_sfx_gain) < 0.01f) { return; }
		m_sfx_gain = gain;
		for (auto& source : m_sfx_sources) {
			if (source.source.state() != capo::State::ePlaying) { continue; }
			source.source.set_gain(m_sfx_gain);
		}
	}

	void play_sfx(capo::Clip const& clip) final {
		if (clip.samples.empty()) { return; }
		auto* source = get_idle_source();
		if (source == nullptr) { source = &get_oldest_source(); }
		play_sfx(*source, clip);
	}

	[[nodiscard]] auto create_stream_source() const -> capo::StreamSource final { return m_device.make_stream_source(); }

	void start_music(capo::StreamSource& source, capo::Clip const& clip) const final {
		source.stop();
		if (clip.samples.empty()) { return; }
		source.set_stream(clip);
		source.play();
	}

  private:
	using Clock = std::chrono::steady_clock;

	struct SfxSource {
		capo::SoundSource source{};
		Clock::time_point timestamp{};
	};

	[[nodiscard]] auto get_idle_source() -> SfxSource* {
		for (auto& source : m_sfx_sources) {
			if (source.source.state() != capo::State::ePlaying) { return &source; }
		}
		return nullptr;
	}

	[[nodiscard]] auto get_oldest_source() -> SfxSource& {
		SfxSource* ret{};
		for (auto& source : m_sfx_sources) {
			if (ret == nullptr || source.timestamp < ret->timestamp) { ret = &source; }
		}
		KLIB_ASSERT(ret != nullptr);
		return *ret;
	}

	void play_sfx(SfxSource& source, capo::Clip const& clip) const {
		source.source.stop();
		source.source.set_gain(m_sfx_gain);
		source.source.set_clip(clip);
		source.source.set_looping(false);
		source.source.play();
		source.timestamp = Clock::now();
	}

	capo::Device m_device{};

	std::vector<SfxSource> m_sfx_sources{};
	float m_sfx_gain{1.0f};
};
} // namespace

void Context::OnDestroy::operator()(int /*i*/) const noexcept { log::info("Context shutting down"); }

Context::Context(gsl::not_null<IDataLoader const*> data_loader, CreateInfo const& create_info)
	: m_data_loader(data_loader), m_window(create_info.window, create_info.render_device),
	  m_pass(&m_window.get_render_device(), create_info.framebuffer_samples) {
	auto default_shader = create_shader(std::string{create_info.default_shader_uri.vertex}, std::string{create_info.default_shader_uri.fragment});
	m_resource_pool = std::make_unique<ResourcePool>(&m_window.get_render_device(), std::move(default_shader));
	m_audio = std::make_unique<Audio>(create_info.sfx_buffers);

	auto const supported_modes = m_window.get_render_device().get_supported_present_modes();
	m_supported_vsync.reserve(supported_modes.size());
	for (auto const mode : supported_modes) { m_supported_vsync.push_back(to_vsync(mode)); }

	log::info("Context initialized");
	m_on_destroy = 42;
}

auto Context::framebuffer_size() const -> glm::ivec2 { return glm::vec2{swapchain_size()} * m_render_scale; }

auto Context::set_render_scale(float const scale) -> bool {
	if (scale < min_render_scale_v || scale > max_render_scale_v) { return false; }
	m_render_scale = scale;
	return true;
}

auto Context::get_vsync() const -> Vsync { return to_vsync(m_window.get_render_device().get_present_mode()); }

auto Context::set_vsync(Vsync const vsync) -> bool {
	if (vsync == get_vsync()) { return true; }
	return m_window.get_render_device().set_present_mode(to_mode(vsync));
}

void Context::cancel_window_close() { m_window.cancel_close(); }

auto Context::next_frame() -> vk::CommandBuffer {
	m_cmd = m_window.next_frame();
	++m_fps.counter;
	m_frame_start = kvf::Clock::now();
	return m_cmd;
}

auto Context::begin_render(kvf::Color const clear) -> Renderer {
	if (!m_cmd) { return {}; }
	m_pass.set_clear_color(clear);
	return m_pass.begin_render(*m_resource_pool, m_cmd, framebuffer_size());
}

void Context::present() {
	auto const present_start = kvf::Clock::now();
	m_window.present(m_pass.get_render_target());
	m_cmd = vk::CommandBuffer{};
	update_stats(present_start);
}

auto Context::create_shader(Uri const& vertex, Uri const& fragment) const -> ShaderProgram {
	auto vert = SpirV{};
	auto frag = SpirV{};
	if (!m_data_loader->load_spirv(vert.code, vertex) || !m_data_loader->load_spirv(frag.code, fragment)) {
		log::warn("Context: failed to load SPIR-V: {} / {}", vertex.get_string(), fragment.get_string());
		return {};
	}
	return ShaderProgram{m_pass.get_render_device().get_device(), vert, frag};
}

auto Context::create_render_pass(vk::SampleCountFlagBits const samples) const -> RenderPass { return RenderPass{&m_pass.get_render_device(), samples}; }

auto Context::create_texture(kvf::Bitmap const& bitmap) const -> Texture { return Texture{&m_pass.get_render_device(), bitmap}; }

auto Context::create_tilesheet(kvf::Bitmap const& bitmap) const -> TileSheet { return TileSheet{&m_pass.get_render_device(), bitmap}; }

auto Context::create_font(std::vector<std::byte> font_bytes) const -> Font { return Font{&m_pass.get_render_device(), std::move(font_bytes)}; }

auto Context::create_asset_load_task(gsl::not_null<klib::task::Queue*> task_queue) const -> std::unique_ptr<asset::LoadTask> {
	auto ret = std::make_unique<asset::LoadTask>(task_queue);
	ret->add_loader(std::make_unique<asset::JsonLoader>(this));
	ret->add_loader(std::make_unique<asset::SpirVLoader>(this));
	ret->add_loader(std::make_unique<asset::FontLoader>(this));
	ret->add_loader(std::make_unique<asset::TileSetLoader>(this));
	ret->add_loader(std::make_unique<asset::TextureLoader>(this));
	ret->add_loader(std::make_unique<asset::TileSheetLoader>(this));
	ret->add_loader(std::make_unique<asset::TransformAnimationLoader>(this));
	ret->add_loader(std::make_unique<asset::TileAnimationLoader>(this));
	ret->add_loader(std::make_unique<asset::PcmLoader>(this));
	return ret;
}

void Context::update_stats(kvf::Clock::time_point const present_start) {
	auto const now = kvf::Clock::now();
	m_frame_stats.present_dt = now - present_start;
	m_frame_stats.total_dt = now - m_frame_start;
	m_fps.elapsed += m_frame_stats.total_dt;
	if (m_fps.elapsed >= 1s) {
		m_fps.value = std::exchange(m_fps.counter, {});
		m_fps.elapsed = {};
	}
	m_frame_stats.framerate = m_fps.value == 0 ? m_fps.counter : m_fps.value;
	++m_frame_stats.total_frames;
	m_frame_stats.run_time = now - m_runtime_start;
}
} // namespace le
