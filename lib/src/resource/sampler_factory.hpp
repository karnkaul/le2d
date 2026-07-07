#pragma once
#include "klib/hash_combine.hpp"
#include "kvf/render_device.hpp"
#include "kvf/util.hpp"
#include "le2d/resource/sampler_factory.hpp"
#include <vulkan/vulkan_hash.hpp>
#include <unordered_map>

namespace le::detail {
class SamplerFactory : public ISamplerFactory {
  public:
	explicit SamplerFactory(gsl::not_null<kvf::IRenderDevice*> render_device) : m_render_device(render_device) {}

  private:
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

	struct Hash {
		auto operator()(TextureSampler const& sampler) const -> std::size_t { return klib::make_combined_hash(sampler.border, sampler.filter, sampler.wrap); }
	};

	std::unordered_map<TextureSampler, vk::UniqueSampler, Hash> m_map{};
	gsl::not_null<kvf::IRenderDevice*> m_render_device;
};
} // namespace le::detail
