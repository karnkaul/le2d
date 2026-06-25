#pragma once
#include "kvf/render_api.hpp"
#include "le2d/resource/resource_factory.hpp"
#include <detail/resource/audio_buffer.hpp>
#include <detail/resource/font.hpp>
#include <detail/resource/shader.hpp>
#include <detail/resource/texture.hpp>
#include <gsl/pointers>

namespace le::detail {
class ResourceFactory : public IResourceFactory {
  public:
	explicit ResourceFactory(gsl::not_null<kvf::IRenderApi const*> render_api, gsl::not_null<ISamplerFactory*> sampler_factory)
		: m_render_api(render_api), m_sampler_factory(sampler_factory) {}

  private:
	[[nodiscard]] auto create_shader() const -> std::unique_ptr<IShader> final { return std::make_unique<Shader>(m_render_api->get_device()); }

	[[nodiscard]] auto create_texture(TextureSampler const& sampler) const -> std::unique_ptr<ITexture> final {
		return std::make_unique<Texture>(m_render_api, m_sampler_factory, sampler);
	}

	[[nodiscard]] auto create_tilesheet(TextureSampler const& sampler) const -> std::unique_ptr<ITileSheet> final {
		return std::make_unique<TileSheet>(m_render_api, m_sampler_factory, sampler);
	}

	[[nodiscard]] auto create_font() const -> std::unique_ptr<IFont> final { return std::make_unique<Font>(m_render_api, m_sampler_factory); }

	[[nodiscard]] auto create_audio_buffer() const -> std::unique_ptr<IAudioBuffer> final { return std::make_unique<AudioBuffer>(); }

	gsl::not_null<kvf::IRenderApi const*> m_render_api;
	gsl::not_null<ISamplerFactory*> m_sampler_factory;
};
} // namespace le::detail
