#include <klib/hash_combine.hpp>
#include <kvf/render_device.hpp>
#include <le2d/resource_pools/sampler_pool.hpp>
#include <vulkan/vulkan_hash.hpp>

namespace le {
auto SamplerPool::Hasher::operator()(TextureSampler const& sampler) const -> std::size_t {
	return klib::make_combined_hash(sampler.wrap_u, sampler.wrap_v, sampler.min_filter, sampler.mag_filter, sampler.mip_map);
}

auto SamplerPool::allocate(TextureSampler const& sampler) -> vk::Sampler {
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
} // namespace le
