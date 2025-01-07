#pragma once
#include <le2d/data_loader.hpp>
#include <le2d/uri.hpp>
#include <vulkan/vulkan.hpp>

namespace le {
class Shader {
  public:
	struct Modules {
		vk::ShaderModule vertex{};
		vk::ShaderModule fragment{};
	};

	Shader() = default;

	explicit Shader(IDataLoader const& data_loader, vk::Device device, Uri const& vertex, Uri const& fragment);

	auto load(IDataLoader const& data_loader, vk::Device device, Uri const& vertex, Uri const& fragment) -> bool;

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
