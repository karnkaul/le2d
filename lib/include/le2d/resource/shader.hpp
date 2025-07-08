#pragma once
#include <le2d/resource/resource.hpp>
#include <vulkan/vulkan.hpp>

namespace le {
class IShader : public IResource {
  public:
	using SpirV = std::span<std::uint32_t const>;

	struct Modules {
		vk::ShaderModule vertex{};
		vk::ShaderModule fragment{};
	};

	[[nodiscard]] virtual auto load(SpirV vertex, SpirV fragment) -> bool = 0;

	[[nodiscard]] virtual auto get_modules() const -> Modules = 0;
	[[nodiscard]] virtual auto get_hash() const -> std::size_t = 0;
};
} // namespace le
