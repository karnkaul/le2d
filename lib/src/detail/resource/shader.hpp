#pragma once
#include "le2d/resource/shader.hpp"
#include <klib/hash_combine.hpp>

namespace le::detail {
class Shader : public IShader {
  public:
	explicit Shader(vk::Device const device) : m_device(device) {}

	[[nodiscard]] auto is_ready() const -> bool final { return m_vertex && m_fragment; }

	[[nodiscard]] auto load(SpirV vertex, SpirV fragment) -> bool final {
		auto smci = std::array<vk::ShaderModuleCreateInfo, 2>{};
		smci[0].setCode(vertex);
		smci[1].setCode(fragment);
		auto vert = m_device.createShaderModuleUnique(smci[0]);
		auto frag = m_device.createShaderModuleUnique(smci[1]);
		if (!vert || !frag) { return false; }

		m_vertex = std::move(vert);
		m_fragment = std::move(frag);
		m_hash = compute_hash(vertex, fragment);

		return true;
	}

	[[nodiscard]] auto get_modules() const -> Modules final { return Modules{.vertex = *m_vertex, .fragment = *m_fragment}; }
	[[nodiscard]] auto get_hash() const -> std::size_t final { return m_hash; }

  private:
	[[nodiscard]] static auto compute_hash(SpirV const vert, SpirV const frag) -> std::size_t {
		auto ret = std::size_t{};
		auto const compute = [&](SpirV const spir_v) {
			for (auto const u32 : spir_v) { klib::hash_combine(ret, u32); }
		};
		compute(vert);
		compute(frag);
		return ret;
	}

	vk::Device m_device{};

	vk::UniqueShaderModule m_vertex{};
	vk::UniqueShaderModule m_fragment{};
	std::size_t m_hash{};
};
} // namespace le::detail
