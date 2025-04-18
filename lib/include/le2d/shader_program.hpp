#pragma once
#include <le2d/asset/spir_v.hpp>
#include <vulkan/vulkan.hpp>

namespace le {
class ShaderProgram {
  public:
	struct Modules {
		vk::ShaderModule vertex{};
		vk::ShaderModule fragment{};
	};

	ShaderProgram() = default;

	explicit ShaderProgram(vk::Device device, SpirV const& vertex, SpirV const& fragment);

	auto load(vk::Device device, SpirV const& vertex, SpirV const& fragment) -> bool;

	[[nodiscard]] auto is_loaded() const -> bool { return m_vertex && m_fragment; }
	[[nodiscard]] auto get_modules() const -> Modules { return Modules{.vertex = *m_vertex, .fragment = *m_fragment}; }
	[[nodiscard]] auto get_hash() const -> std::size_t { return m_hash; }

	explicit operator bool() const { return is_loaded(); }

  private:
	vk::UniqueShaderModule m_vertex{};
	vk::UniqueShaderModule m_fragment{};
	std::size_t m_hash{};
};
} // namespace le
