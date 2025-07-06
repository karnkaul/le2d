#include <capo/engine.hpp>
#include <detail/audio_mixer.hpp>
#include <detail/pipeline_pool.hpp>
#include <detail/resource/resource_factory.hpp>
#include <klib/assert.hpp>
#include <le2d/context.hpp>
#include <log.hpp>
#include <spirv.hpp>

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

[[nodiscard]] auto create_default_shader(vk::Device const device) -> ShaderProgram {
	auto const vert_spirv = spirv::vert();
	auto const frag_spirv = spirv::frag();
	auto ret = ShaderProgram{device, vert_spirv, frag_spirv};
	KLIB_ASSERT(ret.is_loaded());
	return ret;
}

struct ResourcePool : IResourcePool {
	explicit ResourcePool(gsl::not_null<kvf::RenderDevice*> render_device, ShaderProgram default_shader)
		: m_pipelines(render_device), m_default_shader(std::move(default_shader)), m_white_texture(render_device), m_blocker(render_device->get_device()) {}

	[[nodiscard]] auto allocate_pipeline(PipelineFixedState const& state, ShaderProgram const& shader) -> vk::Pipeline final {
		return m_pipelines.allocate(state, shader);
	}

	[[nodiscard]] auto get_pipeline_layout() const -> vk::PipelineLayout final { return m_pipelines.get_layout(); }
	[[nodiscard]] auto get_set_layouts() const -> std::span<vk::DescriptorSetLayout const> final { return m_pipelines.get_set_layouts(); }
	[[nodiscard]] auto get_white_texture() const -> ITexture const& final { return m_white_texture; }
	[[nodiscard]] auto get_default_shader() const -> ShaderProgram const& final { return m_default_shader; }

  private:
	detail::PipelinePool m_pipelines;
	ShaderProgram m_default_shader{};

	detail::Texture m_white_texture;

	kvf::DeviceBlock m_blocker;
};

// TODO: DRY
struct Audio : IAudio {
	explicit Audio(int sfx_sources) : m_engine(capo::create_engine()) {
		if (!m_engine) {
			log.error("Failed to create Audio Engine");
			return;
		}

		static constexpr auto max_sfx_sources_v{256};
		sfx_sources = std::clamp(sfx_sources, 1, max_sfx_sources_v);
		m_sfx_sources.reserve(std::size_t(sfx_sources));
		for (int i = 0; i < sfx_sources; ++i) {
			auto& sfx_source = m_sfx_sources.emplace_back();
			sfx_source.source = m_engine->create_source();
		}
	}

	[[nodiscard]] auto get_sfx_gain() const -> float final { return m_sfx_gain; }

	void set_sfx_gain(float const gain) final {
		if (!m_engine || std::abs(gain - m_sfx_gain) < 0.01f) { return; }
		m_sfx_gain = gain;
		for (auto& source : m_sfx_sources) {
			if (!source.source->is_playing()) { continue; }
			source.source->set_gain(m_sfx_gain);
		}
	}

	void play_sfx(gsl::not_null<capo::Buffer const*> buffer) final {
		if (!m_engine || buffer->get_samples().empty()) { return; }
		auto* source = get_idle_source();
		if (source == nullptr) { source = &get_oldest_source(); }
		play_sfx(*source, [buffer](capo::ISource& source) { source.bind_to(buffer); });
	}

	void play_sfx(std::shared_ptr<capo::Buffer const> buffer) final {
		if (!m_engine || !buffer || buffer->get_samples().empty()) { return; }
		auto* source = get_idle_source();
		if (source == nullptr) { source = &get_oldest_source(); }
		play_sfx(*source, [&buffer](capo::ISource& source) { source.bind_to(std::move(buffer)); });
	}

	[[nodiscard]] auto create_source() const -> std::unique_ptr<capo::ISource> final {
		if (!m_engine) { return {}; }
		return m_engine->create_source();
	}

	void start_music(capo::ISource& source, gsl::not_null<capo::Buffer const*> buffer) const final {
		source.stop();
		if (buffer->get_samples().empty()) { return; }
		source.bind_to(buffer);
		source.play();
	}

	void start_music(capo::ISource& source, std::shared_ptr<capo::Buffer const> buffer) const final {
		source.stop();
		if (!buffer || buffer->get_samples().empty()) { return; }
		source.bind_to(std::move(buffer));
		source.play();
	}

	void wait_idle() final {
		for (auto& source : m_sfx_sources) {
			if (source.source->is_playing()) { source.source->wait_until_ended(); }
			source.source->stop();
		}
	}

  private:
	using Clock = std::chrono::steady_clock;

	struct SfxSource {
		std::unique_ptr<capo::ISource> source{};
		Clock::time_point timestamp{};
	};

	[[nodiscard]] auto get_idle_source() -> SfxSource* {
		for (auto& source : m_sfx_sources) {
			if (!source.source->is_playing()) { return &source; }
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

	template <typename F>
	void play_sfx(SfxSource& source, F func) const {
		source.source->stop();
		source.source->set_gain(m_sfx_gain);
		func(*source.source);
		source.source->set_looping(false);
		source.source->play();
		source.timestamp = Clock::now();
	}

	std::unique_ptr<capo::IEngine> m_engine{};

	std::vector<SfxSource> m_sfx_sources{};
	float m_sfx_gain{1.0f};
};
} // namespace

void Context::OnDestroy::operator()(Context* ptr) const noexcept {
	log.info("Context shutting down");
	ptr->wait_idle();
}

Context::Context(CreateInfo const& create_info)
	: m_window(create_info.window, create_info.render_device), m_pass(&m_window.get_render_device(), create_info.framebuffer_samples),
	  m_resource_factory(std::make_unique<detail::ResourceFactory>(&m_window.get_render_device())), m_blocker(m_window.get_render_device().get_device()) {

	auto default_shader = create_default_shader(m_window.get_render_device().get_device());
	m_resource_pool = std::make_unique<ResourcePool>(&m_window.get_render_device(), std::move(default_shader));
	m_audio = std::make_unique<Audio>(create_info.sfx_buffers);
	m_audio_mixer = std::make_unique<detail::AudioMixer>(create_info.sfx_buffers);

	auto const supported_modes = m_window.get_render_device().get_supported_present_modes();
	m_supported_vsync.reserve(supported_modes.size());
	for (auto const mode : supported_modes) { m_supported_vsync.push_back(to_vsync(mode)); }

	log.info("Context initialized");
	m_on_destroy.reset(this);
}

auto Context::get_latest_gamepad() -> Gamepad const& {
	if (!m_latest_gamepad.is_connected()) {
		m_latest_gamepad = Gamepad::get_active();
	} else {
		m_latest_gamepad = Gamepad::get_by_id(m_latest_gamepad.get_id());
	}
	return m_latest_gamepad;
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

void Context::wait_idle() {
	log.debug("Context: waiting idle");
	m_window.get_render_device().get_device().waitIdle();
	// TODO: remove
	static_cast<Audio&>(*m_audio).wait_idle();
	m_audio_mixer->wait_idle();
}

auto Context::create_shader(SpirV const vertex, SpirV const fragment) const -> ShaderProgram {
	return ShaderProgram{m_pass.get_render_device().get_device(), vertex, fragment};
}

auto Context::create_render_pass(vk::SampleCountFlagBits const samples) const -> RenderPass { return RenderPass{&m_pass.get_render_device(), samples}; }

auto Context::create_texture(kvf::Bitmap const& bitmap) const -> Texture { return Texture{&m_pass.get_render_device(), bitmap}; }

auto Context::create_tilesheet(kvf::Bitmap const& bitmap) const -> TileSheet { return TileSheet{&m_pass.get_render_device(), bitmap}; }

auto Context::create_font(std::vector<std::byte> font_bytes) const -> Font { return Font{&m_pass.get_render_device(), std::move(font_bytes)}; }

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
