#pragma once
#include "le2d/vertex.hpp"
#include <klib/base_types.hpp>
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace le {
/// \brief Interface for drawable geometry.
class IGeometry : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_vertices() const -> std::span<Vertex const> = 0;
	[[nodiscard]] virtual auto get_indices() const -> std::span<std::uint32_t const> = 0;
	[[nodiscard]] virtual auto get_topology() const -> vk::PrimitiveTopology = 0;
};
} // namespace le
