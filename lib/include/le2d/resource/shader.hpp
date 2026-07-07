#pragma once
#include "kvf/graphics_shader.hpp"
#include "le2d/resource/resource.hpp"

namespace le {
/// \brief Opaque interface for a Shader program.
class IShader : public IResource {
  public:
	using SpirV = std::span<std::uint32_t const>;

	[[nodiscard]] virtual auto load(SpirV vertex, SpirV fragment) -> bool = 0;

	[[nodiscard]] virtual auto get_kvf_shader() const -> kvf::IGraphicsShader const& = 0;
};
} // namespace le
