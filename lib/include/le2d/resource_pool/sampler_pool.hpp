#pragma once
#include <kvf/render_device_fwd.hpp>
#include <le2d/texture_sampler.hpp>
#include <gsl/pointers>
#include <unordered_map>

namespace le {
class SamplerPool {
  public:
	explicit SamplerPool(gsl::not_null<kvf::RenderDevice const*> render_device) : m_render_device(render_device) {}

	[[nodiscard]] auto allocate(TextureSampler const& sampler) -> vk::Sampler;

  private:
	struct Hasher {
		[[nodiscard]] auto operator()(TextureSampler const& sampler) const -> std::size_t;
	};

	kvf::RenderDevice const* m_render_device;

	std::unordered_map<TextureSampler, vk::UniqueSampler, Hasher> m_map{};
};
} // namespace le
