#pragma once
#include "detail/render_resources.hpp"
#include <gsl/pointers>

namespace le::detail {
class CachedSampler {
  public:
	explicit CachedSampler(gsl::not_null<ISamplerFactory*> sampler_factory, TextureSampler const& sampler = {}) : m_sampler_factory(sampler_factory) {
		set_sampler(sampler);
	}

	[[nodiscard]] auto get_vk_sampler() const -> vk::Sampler { return m_vk_sampler; }

	[[nodiscard]] auto get_sampler() const -> TextureSampler const& { return m_sampler; }

	void set_sampler(TextureSampler const& sampler) {
		m_sampler = sampler;
		m_vk_sampler = m_sampler_factory->get_or_create(m_sampler);
	}

  private:
	gsl::not_null<ISamplerFactory*> m_sampler_factory;

	TextureSampler m_sampler{};
	vk::Sampler m_vk_sampler{};
};
} // namespace le::detail
