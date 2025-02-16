#pragma once
#include <klib/hash_combine.hpp>
#include <kvf/render_device.hpp>
#include <le2d/texture_sampler.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <gsl/pointers>
#include <unordered_map>

namespace le::detail {
class SamplerPool {
  public:
	explicit SamplerPool(gsl::not_null<kvf::RenderDevice const*> render_device) : m_render_device(render_device) {}

	[[nodiscard]] auto allocate(TextureSampler const& sampler) -> vk::Sampler {
		auto it = m_map.find(sampler);
		if (it == m_map.end()) {
			auto sci = m_render_device->sampler_info({}, {});
			sci.setAddressModeU(sampler.wrap_u)
				.setAddressModeV(sampler.wrap_v)
				.setMinFilter(sampler.min_filter)
				.setMagFilter(sampler.mag_filter)
				.setMipmapMode(sampler.mip_map)
				.setBorderColor(sampler.border_color);
			it = m_map.insert({sampler, m_render_device->get_device().createSamplerUnique(sci)}).first;
		}
		return *it->second;
	}

  private:
	struct Hasher {
		[[nodiscard]] auto operator()(TextureSampler const& sampler) const -> std::size_t {
			return klib::make_combined_hash(sampler.wrap_u, sampler.wrap_v, sampler.min_filter, sampler.mag_filter, sampler.mip_map);
		}
	};

	kvf::RenderDevice const* m_render_device;

	std::unordered_map<TextureSampler, vk::UniqueSampler, Hasher> m_map{};
};
} // namespace le::detail
