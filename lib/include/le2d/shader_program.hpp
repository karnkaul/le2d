#pragma once
#include <vulkan/vulkan.hpp>
#include <cstdint>
#include <span>

namespace le {
class ShaderProgram {
  public:
	using SpirV = std::span<std::uint32_t const>;

	struct Modules {
		vk::ShaderModule vertex{};
		vk::ShaderModule fragment{};
	};

	ShaderProgram() = default;

	explicit ShaderProgram(vk::Device device, SpirV vertex, SpirV fragment);

	auto load(vk::Device device, SpirV vertex, SpirV fragment) -> bool;

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
