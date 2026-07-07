#pragma once
#include "detail/resource/audio_buffer.hpp"
#include "detail/resource/font.hpp"
#include "detail/resource/shader.hpp"
#include "detail/resource/texture.hpp"
#include "detail/shader_layout.hpp"
#include "kvf/render_device.hpp"
#include "le2d/resource/resource_factory.hpp"
#include <gsl/pointers>

namespace le::detail {
class ResourceFactory : public IResourceFactory {
  public:
	explicit ResourceFactory(gsl::not_null<kvf::IRenderDevice*> render_device, gsl::not_null<ISamplerFactory*> sampler_factory)
		: m_render_device(render_device), m_sampler_factory(sampler_factory), m_shader_layout(render_device->get_device()) {}

	[[nodiscard]] auto get_render_device() const -> kvf::IRenderDevice& final { return *m_render_device; }
	[[nodiscard]] auto get_sampler_factory() const -> ISamplerFactory& { return *m_sampler_factory; }
	[[nodiscard]] auto get_shader_layout() const -> ShaderLayout const& { return m_shader_layout; }

  private:
	[[nodiscard]] auto create_shader(SpirV const vertex, SpirV const fragment) const -> std::unique_ptr<IShader> final {
		auto ret = std::make_unique<Shader>(m_render_device, m_shader_layout.get_set_layouts());
		if (!ret->load(vertex, fragment)) { return {}; }
		return ret;
	}

	[[nodiscard]] auto create_texture(kvf::Bitmap const bitmap, TextureSampler sampler) const -> std::unique_ptr<ITexture> final {
		return std::make_unique<Texture>(m_render_device, m_sampler_factory, bitmap, sampler);
	}

	[[nodiscard]] auto create_tilesheet(kvf::Bitmap bitmap, TextureSampler sampler) const -> std::unique_ptr<ITileSheet> final {
		return std::make_unique<TileSheet>(m_render_device, m_sampler_factory, bitmap, sampler);
	}

	[[nodiscard]] auto create_font(std::vector<std::byte> font_bytes) const -> std::unique_ptr<IFont> final {
		auto ret = std::make_unique<Font>(m_render_device, m_sampler_factory);
		if (!ret->load_face(std::move(font_bytes))) { return {}; }
		return ret;
	}

	[[nodiscard]] auto create_audio_buffer(std::span<std::byte const> bytes, std::optional<capo::Encoding> encoding) const
		-> std::unique_ptr<IAudioBuffer> final {
		auto ret = std::make_unique<AudioBuffer>();
		if (!ret->decode(bytes, encoding)) { return {}; }
		return ret;
	}

	gsl::not_null<kvf::IRenderDevice*> m_render_device;
	gsl::not_null<ISamplerFactory*> m_sampler_factory;

	ShaderLayout m_shader_layout;
};
} // namespace le::detail
