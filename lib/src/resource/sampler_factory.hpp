#pragma once
#include "klib/hash_combine.hpp"
#include "kvf/render_api.hpp"
#include "kvf/util.hpp"
#include "le2d/resource/sampler_factory.hpp"
#include "le2d/resource/texture.hpp"
#include <vulkan/vulkan_hash.hpp>
#include <mutex>
#include <unordered_map>

namespace le::detail {
class SamplerFactory : public ISamplerFactory {
  public:
	explicit SamplerFactory(gsl::not_null<kvf::IRenderApi const*> render_api) : m_render_api(render_api) {}

  private:
	[[nodiscard]] auto get_or_create(TextureSampler const& sampler) -> vk::Sampler final {
		auto lock = std::scoped_lock{m_mutex};
		auto it = m_map.find(sampler);
		if (it == m_map.end()) {
			auto sampler_ci = kvf::util::create_sampler_ci(sampler.wrap, sampler.filter);
			sampler_ci.setBorderColor(sampler.border);
			auto vk_sampler = m_render_api->create_sampler(sampler_ci);
			it = m_map.insert_or_assign(sampler, std::move(vk_sampler)).first;
		}
		return *it->second;
	}

	struct Hash {
		auto operator()(TextureSampler const& sampler) const -> std::size_t { return klib::make_combined_hash(sampler.border, sampler.filter, sampler.wrap); }
	};

	std::unordered_map<TextureSampler, vk::UniqueSampler, Hash> m_map{};
	std::mutex m_mutex{};
	gsl::not_null<kvf::IRenderApi const*> m_render_api;
};
} // namespace le::detail
