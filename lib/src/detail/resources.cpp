#include "capo/engine.hpp"
#include "detail/cached_sampler.hpp"
#include "detail/context_resources.hpp"
#include "detail/renderer.hpp"
#include "klib/debug/assert.hpp"
#include "klib/hash_combine.hpp"
#include "kvf/device_waiter.hpp"
#include "kvf/image_bitmap.hpp"
#include "kvf/render_device.hpp"
#include "kvf/render_image.hpp"
#include "kvf/render_pass.hpp"
#include "kvf/util.hpp"
#include "le2d/error.hpp"
#include "le2d/text/util.hpp"
#include "log.hpp"
#include "spirv.hpp"

namespace le::detail {
namespace {
#pragma region AudioMixer

class AudioMixer : public IAudioMixer {
  public:
	explicit AudioMixer(int sfx_sources) : m_engine(capo::create_engine()) {
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

		m_deck.primary = m_engine->create_source();
		m_deck.secondary = m_engine->create_source();
	}

  private:
	using Clock = std::chrono::steady_clock;

	struct SfxSource {
		std::unique_ptr<capo::ISource> source{};
		Clock::time_point timestamp{};
	};

	struct Deck {
		std::unique_ptr<capo::ISource> primary{};
		std::unique_ptr<capo::ISource> secondary{};
		float gain{1.0f};
	};

	[[nodiscard]] auto get_sfx_gain() const -> float final { return m_sfx_gain; }

	void set_sfx_gain(float const gain) final {
		if (!m_engine || std::abs(gain - m_sfx_gain) < 0.01f) { return; }
		m_sfx_gain = gain;
		for (auto& source : m_sfx_sources) {
			if (!source.source->is_playing()) { continue; }
			source.source->set_gain(m_sfx_gain);
		}
	}

	void play_sfx(gsl::not_null<IAudioBuffer const*> buffer) final {
		if (!m_engine) { return; }
		auto* source = get_idle_source();
		if (source == nullptr) { source = &get_oldest_source(); }
		play_sfx(*source, [buffer](capo::ISource& source) { buffer->bind(source); });
	}

	void stop_sfx() final {
		for (auto& source : m_sfx_sources) { source.source->unbind(); }
	}

	[[nodiscard]] auto get_music_gain() const -> float final { return m_deck.gain; }

	void set_music_gain(float const gain) final {
		m_deck.gain = gain;
		m_deck.primary->set_gain(gain);
	}

	void loop_music(gsl::not_null<IAudioBuffer const*> buffer, kvf::Seconds const fade) final {
		if (m_deck.primary->is_playing()) {
			m_deck.secondary->stop();
			m_deck.primary->set_looping(false);
			m_deck.primary->set_fade_out(fade);
			std::swap(m_deck.primary, m_deck.secondary);
		}
		m_deck.primary->stop();
		if (!buffer->bind(*m_deck.primary)) { return; }
		m_deck.primary->set_looping(true);
		m_deck.primary->set_fade_in(fade, m_deck.gain);
		m_deck.primary->play();
	}

	void pause_music() final {
		auto const pause = [](capo::ISource& source) { source.stop(); };
		pause(*m_deck.primary);
		pause(*m_deck.secondary);
	}

	void resume_music() final {
		auto const resume = [](capo::ISource& source) { source.play(); };
		resume(*m_deck.primary);
		resume(*m_deck.secondary);
	}

	void stop_music() final {
		auto const stop = [](capo::ISource& source) { source.unbind(); };
		stop(*m_deck.primary);
		stop(*m_deck.secondary);
	}

	[[nodiscard]] auto create_source() const -> std::unique_ptr<capo::ISource> final {
		if (!m_engine) { return {}; }
		return m_engine->create_source();
	}

	[[nodiscard]] auto is_playing() const -> bool final {
		if (std::ranges::any_of(m_sfx_sources, [](SfxSource const& s) { return s.source->is_playing(); })) { return true; }
		return m_deck.primary->is_playing() || m_deck.secondary->is_playing();
	}

	void wait_idle() final {
		for (auto& source : m_sfx_sources) {
			if (source.source->is_playing()) { source.source->wait_until_ended(); }
			source.source->unbind();
		}
		stop_music();
	}

	void stop_all() final {
		stop_sfx();
		stop_music();
	}

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
	Deck m_deck{};
	float m_sfx_gain{1.0f};
};

#pragma endregion

#pragma region AudioBuffer

class AudioBuffer : public IAudioBuffer {
  public:
	explicit AudioBuffer() = default;

	auto decode(std::span<std::byte const> bytes, std::optional<capo::Encoding> const encoding) -> bool final { return m_buffer.decode_bytes(bytes, encoding); }

  private:
	[[nodiscard]] auto get_duration() const -> kvf::Seconds final {
		KLIB_ASSERT(m_buffer.is_loaded());
		auto const samples = float(m_buffer.get_samples().size());
		return kvf::Seconds{float(capo::Buffer::sample_rate_v) * (samples / float(m_buffer.get_channels()))};
	}

	auto bind(capo::ISource& source) const -> bool final {
		KLIB_ASSERT(m_buffer.is_loaded());
		return source.bind_to(&m_buffer);
	}

	capo::Buffer m_buffer{};
};

#pragma endregion

#pragma region SamplerFactory

class SamplerFactory : public ISamplerFactory {
  public:
	explicit SamplerFactory(gsl::not_null<kvf::IRenderDevice*> render_device) : m_render_device(render_device) {}

  private:
	struct Hash {
		auto operator()(TextureSampler const& sampler) const -> std::size_t { return klib::make_combined_hash(sampler.border, sampler.filter, sampler.wrap); }
	};

	[[nodiscard]] auto get_render_device() const -> kvf::IRenderDevice& final { return *m_render_device; }

	[[nodiscard]] auto get_or_create(TextureSampler const& sampler) -> vk::Sampler final {
		auto it = m_map.find(sampler);
		if (it == m_map.end()) {
			auto sampler_ci = kvf::util::create_sampler_ci(sampler.wrap, sampler.filter);
			sampler_ci.setBorderColor(sampler.border);
			auto vk_sampler = m_render_device->create_sampler(sampler_ci);
			it = m_map.insert_or_assign(sampler, std::move(vk_sampler)).first;
		}
		return *it->second;
	}

	std::unordered_map<TextureSampler, vk::UniqueSampler, Hash> m_map{};
	gsl::not_null<kvf::IRenderDevice*> m_render_device;
};

#pragma endregion

#pragma region Texture

class TextureBase {
  public:
	explicit TextureBase(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory, kvf::Bitmap const& bitmap,
						 TextureSampler const& sampler)
		: m_render_device(render_device), m_texture(kvf::IRenderImage::create_texture(render_device, bitmap)), m_cached_sampler(sampler_factory, sampler) {
		set_sampler(sampler);
	}

	[[nodiscard]] auto get_image() const -> vk::ImageView { return m_texture->get_image_view(); }

	[[nodiscard]] auto get_sampler() const -> TextureSampler const& { return m_cached_sampler.get_sampler(); }
	void set_sampler(TextureSampler const& sampler) { m_cached_sampler.set_sampler(sampler); }

	[[nodiscard]] auto get_size() const -> glm::ivec2 { return kvf::util::to_glm_vec<int>(m_texture->get_extent()); }

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo { return m_texture->descriptor_info(m_cached_sampler.get_vk_sampler()); }

	void overwrite(kvf::Bitmap const& bitmap) { m_texture->resize_and_overwrite(bitmap); }

	auto load_and_write(std::span<std::byte const> compressed_image) -> bool {
		auto const image = kvf::ImageBitmap{compressed_image};
		if (!image.is_loaded()) { return false; }
		overwrite(image.bitmap());
		return true;
	}

  private:
	gsl::not_null<kvf::IRenderDevice*> m_render_device;

	std::unique_ptr<kvf::IRenderImage> m_texture;
	CachedSampler m_cached_sampler;
};

template <std::derived_from<ITextureBase> BaseT>
class TextureImpl : public BaseT {
  public:
	explicit TextureImpl(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory, kvf::Bitmap const& bitmap = {},
						 TextureSampler const& sampler = {})
		: m_base(render_device, sampler_factory, bitmap, sampler) {}

	[[nodiscard]] auto get_image() const -> vk::ImageView final { return m_base.get_image(); }
	[[nodiscard]] auto get_size() const -> glm::ivec2 final { return m_base.get_size(); }

	[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final { return m_base.descriptor_info(); }

	void overwrite(kvf::Bitmap const& bitmap) final { m_base.overwrite(bitmap); }
	auto load_and_write(std::span<std::byte const> compressed_image) -> bool final { return m_base.load_and_write(compressed_image); }

	[[nodiscard]] auto get_sampler() const -> TextureSampler const& final { return m_base.get_sampler(); }
	void set_sampler(TextureSampler const& sampler) final { m_base.set_sampler(sampler); }

  private:
	TextureBase m_base;
};

using Texture = TextureImpl<ITexture>;
using TileSheet = TextureImpl<ITileSheet>;

#pragma endregion

#pragma region Font

class FontAtlas : public IFontAtlas {
  public:
	using Glyph = kvf::ttf::Glyph;
	using GlyphLayout = kvf::ttf::GlyphLayout;

	explicit FontAtlas(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory)
		: m_texture(render_device, sampler_factory) {}

	void build(gsl::not_null<kvf::ttf::Typeface*> face, TextHeight height) {
		height = util::clamp(height);
		auto ttf_atlas = face->build_atlas(std::uint32_t(height));
		m_texture.overwrite(ttf_atlas.bitmap.bitmap());

		m_face = face;
		m_height = height;
		m_glyphs = std::move(ttf_atlas.glyphs);
	}

  private:
	[[nodiscard]] auto get_glyphs() const -> std::span<Glyph const> final { return m_glyphs; }
	[[nodiscard]] auto get_texture() const -> ITexture const& final { return m_texture; }
	[[nodiscard]] auto get_height() const -> TextHeight final { return m_height; }

	auto push_layouts(std::vector<GlyphLayout>& out, std::string_view const text, float const n_line_height, bool const use_tofu) const -> glm::vec2 final {
		auto const input = kvf::ttf::TextInput{
			.text = text,
			.glyphs = m_glyphs,
			.height = std::uint32_t(m_height),
			.n_line_height = n_line_height,
		};
		return m_face->push_layouts(out, input, use_tofu);
	}

	kvf::ttf::Typeface* m_face{};
	Texture m_texture;
	std::vector<Glyph> m_glyphs{};
	TextHeight m_height{};
};

class Font : public IFont {
  public:
	explicit Font(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory)
		: m_render_device(render_device), m_sampler_factory(sampler_factory) {}

	auto load_face(std::vector<std::byte> font_bytes) -> bool final {
		auto face = kvf::ttf::Typeface{std::move(font_bytes)};
		if (!face) { return false; }

		m_face = std::move(face);
		m_atlases.clear();

		return true;
	}

	[[nodiscard]] auto get_name() const -> klib::CString final {
		KLIB_ASSERT(m_face.is_loaded());
		return m_face.get_name();
	}

	[[nodiscard]] auto get_atlas(TextHeight height) -> FontAtlas& final {
		KLIB_ASSERT(m_face.is_loaded());
		height = util::clamp(height);
		auto it = m_atlases.find(height);
		if (it == m_atlases.end()) {
			auto atlas = FontAtlas{m_render_device, m_sampler_factory};
			atlas.build(&m_face, height);
			it = m_atlases.insert({height, std::move(atlas)}).first;
		}
		return it->second;
	}

  private:
	gsl::not_null<kvf::IRenderDevice*> m_render_device;
	gsl::not_null<ISamplerFactory*> m_sampler_factory;

	kvf::ttf::Typeface m_face{};
	std::unordered_map<TextHeight, FontAtlas> m_atlases{};
};

#pragma endregion

#pragma region Shader

class Shader : public IShader {
  public:
	explicit Shader(gsl::not_null<kvf::IRenderDevice*> render_device, std::span<vk::DescriptorSetLayout const> set_layouts)
		: m_render_device(render_device), m_set_layouts(set_layouts) {}

	[[nodiscard]] auto load(SpirV vertex, SpirV fragment) -> bool final {
		static constexpr auto bindings_v = std::array{
			vk::VertexInputBindingDescription2EXT{0, sizeof(Vertex), vk::VertexInputRate::eVertex, 1},
		};

		static constexpr auto attributes_v = std::array{
			vk::VertexInputAttributeDescription2EXT{0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)},
			vk::VertexInputAttributeDescription2EXT{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
			vk::VertexInputAttributeDescription2EXT{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
		};

		static constexpr auto input_v = kvf::GraphicsShaderInput{
			.bindings = bindings_v,
			.attributes = attributes_v,
		};

		auto const shader_ci = kvf::IGraphicsShader::CreateInfo{
			.code = {.vertex = vertex, .fragment = fragment},
			.input = input_v,
			.set_layouts = m_set_layouts,
		};
		m_shader = kvf::IGraphicsShader::create(m_render_device, shader_ci);
		return m_shader != nullptr;
	}

  private:
	[[nodiscard]] auto get_kvf_shader() const -> kvf::IGraphicsShader const& final {
		KLIB_ASSERT(m_shader);
		return *m_shader;
	}

	gsl::not_null<kvf::IRenderDevice*> m_render_device;
	std::span<vk::DescriptorSetLayout const> m_set_layouts;

	std::unique_ptr<kvf::IGraphicsShader> m_shader{};
};

#pragma endregion

#pragma region ResourceFactory

class ResourceFactory : public IResourceFactory {
  public:
	explicit ResourceFactory(gsl::not_null<ISamplerFactory*> sampler_factory, gsl::not_null<ShaderLayout const*> shader_layout)
		: m_sampler_factory(sampler_factory), m_shader_layout(shader_layout) {}

  private:
	[[nodiscard]] auto get_render_device() const -> kvf::IRenderDevice& final { return m_sampler_factory->get_render_device(); }

	[[nodiscard]] auto create_shader(SpirV const vertex, SpirV const fragment) const -> std::unique_ptr<IShader> final {
		auto ret = std::make_unique<Shader>(&get_render_device(), m_shader_layout->get_set_layouts());
		if (!ret->load(vertex, fragment)) { return {}; }
		return ret;
	}

	[[nodiscard]] auto create_texture(kvf::Bitmap const bitmap, TextureSampler sampler) const -> std::unique_ptr<ITexture> final {
		return std::make_unique<Texture>(&get_render_device(), m_sampler_factory, bitmap, sampler);
	}

	[[nodiscard]] auto create_tilesheet(kvf::Bitmap bitmap, TextureSampler sampler) const -> std::unique_ptr<ITileSheet> final {
		return std::make_unique<TileSheet>(&get_render_device(), m_sampler_factory, bitmap, sampler);
	}

	[[nodiscard]] auto create_font(std::vector<std::byte> font_bytes) const -> std::unique_ptr<IFont> final {
		auto ret = std::make_unique<Font>(&get_render_device(), m_sampler_factory);
		if (!ret->load_face(std::move(font_bytes))) { return {}; }
		return ret;
	}

	[[nodiscard]] auto create_audio_buffer(std::span<std::byte const> bytes, std::optional<capo::Encoding> encoding) const
		-> std::unique_ptr<IAudioBuffer> final {
		auto ret = std::make_unique<AudioBuffer>();
		if (!ret->decode(bytes, encoding)) { return {}; }
		return ret;
	}

	gsl::not_null<ISamplerFactory*> m_sampler_factory;
	gsl::not_null<ShaderLayout const*> m_shader_layout;
};

#pragma endregion

#pragma region RenderResources

[[nodiscard]] auto create_default_shader(gsl::not_null<IResourceFactory const*> resource_factory) -> std::unique_ptr<IShader> {
	auto const vert_spirv = spirv::vert();
	auto const frag_spirv = spirv::frag();
	auto ret = resource_factory->create_shader(vert_spirv, frag_spirv);
	if (!ret->load(vert_spirv, frag_spirv)) { throw Error{"Failed to create default shader"}; }
	return ret;
}

class RenderResources : public IRenderResources {
  public:
	explicit RenderResources(gsl::not_null<ISamplerFactory*> sampler_factory, gsl::not_null<ShaderLayout const*> shader_layout,
							 gsl::not_null<IResourceFactory const*> resource_factory)
		: m_shader_layout(shader_layout), m_default_shader(create_default_shader(resource_factory)),
		  m_white_texture(&resource_factory->get_render_device(), sampler_factory), m_waiter(resource_factory->get_render_device().get_device()) {}

	[[nodiscard]] auto get_shader_layout() const -> ShaderLayout const& final { return *m_shader_layout; }
	[[nodiscard]] auto get_default_shader() const -> IShader const& final { return *m_default_shader; }
	[[nodiscard]] auto get_white_texture() const -> ITexture const& final { return m_white_texture; }

	std::vector<RenderInstance::Std430> render_instance_buffer{};

  private:
	gsl::not_null<ShaderLayout const*> m_shader_layout;

	std::unique_ptr<IShader> m_default_shader{};

	Texture m_white_texture;

	kvf::DeviceWaiter m_waiter;
};

#pragma endregion

#pragma region RenderPass

class RenderPass : public IRenderPass {
  public:
	static constexpr auto color_format_v{vk::Format::eR8G8B8A8Srgb};

	static constexpr auto clamp_size(glm::ivec2 in) {
		in.x = std::clamp(in.x, RenderPass::min_size_v, RenderPass::max_size_v);
		in.y = std::clamp(in.y, RenderPass::min_size_v, RenderPass::max_size_v);
		return in;
	}

	explicit RenderPass(gsl::not_null<ISamplerFactory*> sampler_factory, gsl::not_null<IRenderResources*> resources, vk::SampleCountFlagBits samples)
		: m_render_device(&sampler_factory->get_render_device()), m_resources(resources), m_render_pass(kvf::IRenderPass::create(m_render_device, samples)),
		  m_render_texture(std::make_unique<RenderTexture>(sampler_factory, m_render_pass.get(), &m_resources->get_white_texture())),
		  m_waiter(m_render_device->get_device()) {
		m_render_pass->set_color_target(color_format_v);
	}

	[[nodiscard]] auto get_render_device() const -> kvf::IRenderDevice& final { return *m_render_device; }

	[[nodiscard]] auto get_render_target() const -> kvf::RenderTarget const& final { return m_render_pass->render_target(); }

	[[nodiscard]] auto get_samples() const -> vk::SampleCountFlagBits final { return m_render_pass->get_samples(); }

	[[nodiscard]] auto render_texture() const -> IRenderTexture& final { return *m_render_texture; }

	void set_clear_color(kvf::Color const color) final { m_render_pass->clear_color = color.to_linear(); }

	void recreate(vk::SampleCountFlagBits const samples) final { m_render_pass->recreate(samples); }

	[[nodiscard]] auto create_renderer() -> std::unique_ptr<IRenderer> final { return std::make_unique<Renderer>(m_render_pass.get(), m_resources); }

  private:
	class RenderTexture : public IRenderTexture {
	  public:
		explicit RenderTexture(gsl::not_null<ISamplerFactory*> sampler_factory, gsl::not_null<kvf::IRenderPass*> render_pass,
							   gsl::not_null<ITexture const*> fallback_texture)
			: m_render_pass(render_pass), m_fallback_texture(fallback_texture), m_cached_sampler(sampler_factory) {}

	  private:
		[[nodiscard]] auto get_image() const -> vk::ImageView final { return get_target_image(); }

		[[nodiscard]] auto get_size() const -> glm::ivec2 final { return kvf::util::to_glm_vec(m_render_pass->get_extent()); }

		[[nodiscard]] auto get_sampler() const -> TextureSampler const& final { return m_cached_sampler.get_sampler(); }
		void set_sampler(TextureSampler const& sampler) final { m_cached_sampler.set_sampler(sampler); }

		[[nodiscard]] auto descriptor_info() const -> vk::DescriptorImageInfo final {
			auto const ret = m_render_pass->render_texture_descriptor_info(m_cached_sampler.get_vk_sampler());
			return ret.value_or(m_fallback_texture->descriptor_info());
		}

		[[nodiscard]] auto has_render_target() const -> bool final { return get_target_image(); }

		[[nodiscard]] auto get_target_image() const -> vk::ImageView { return m_render_pass->render_target().view; }

		gsl::not_null<kvf::IRenderPass*> m_render_pass;
		gsl::not_null<ITexture const*> m_fallback_texture;
		CachedSampler m_cached_sampler;
	};

	gsl::not_null<kvf::IRenderDevice*> m_render_device;
	gsl::not_null<IRenderResources*> m_resources;

	std::unique_ptr<kvf::IRenderPass> m_render_pass;
	std::unique_ptr<RenderTexture> m_render_texture;

	kvf::DeviceWaiter m_waiter;
};

#pragma endregion
} // namespace

ContextResources::ContextResources(gsl::not_null<kvf::IRenderDevice*> render_device, int const sfx_sources)
	: audio_mixer(std::make_unique<AudioMixer>(sfx_sources)), shader_layout(std::make_unique<ShaderLayout>(render_device->get_device())),
	  sampler_factory(std::make_unique<SamplerFactory>(render_device)),
	  resource_factory(std::make_unique<ResourceFactory>(sampler_factory.get(), shader_layout.get())),
	  render_resources(std::make_unique<RenderResources>(sampler_factory.get(), shader_layout.get(), resource_factory.get())) {}

auto ContextResources::create_render_pass(vk::SampleCountFlagBits samples) const -> std::unique_ptr<IRenderPass> {
	return std::make_unique<RenderPass>(sampler_factory.get(), render_resources.get(), samples);
}
} // namespace le::detail
