#pragma once
#include <detail/font.hpp>
#include <detail/shader.hpp>
#include <detail/texture.hpp>
#include <kvf/render_api.hpp>
#include <le2d/resource/resource_factory.hpp>
#include <gsl/pointers>

namespace le::detail {
class ResourceFactory : public IResourceFactory {
  public:
	explicit ResourceFactory(gsl::not_null<kvf::IRenderApi const*> render_api) : m_render_api(render_api) {}

  private:
	[[nodiscard]] auto create_shader() const -> std::unique_ptr<IShader> final { return std::make_unique<Shader>(m_render_api->get_device()); }

	[[nodiscard]] auto create_texture(TextureSampler const& sampler) const -> std::unique_ptr<ITexture2> final {
		return std::make_unique<Texture>(m_render_api, sampler);
	}

	[[nodiscard]] auto create_tilesheet(TextureSampler const& sampler) const -> std::unique_ptr<ITileSheet> final {
		return std::make_unique<TileSheet>(m_render_api, sampler);
	}

	[[nodiscard]] auto create_font() const -> std::unique_ptr<IFont> final { return std::make_unique<Font>(m_render_api); }

	gsl::not_null<kvf::IRenderApi const*> m_render_api;
};
} // namespace le::detail
