#pragma once
#include <klib/base_types.hpp>
#include <le2d/vertex.hpp>
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace le {
class IGeometry : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_vertices() const -> std::span<Vertex const> = 0;
	[[nodiscard]] virtual auto get_indices() const -> std::span<std::uint32_t const> = 0;
	[[nodiscard]] virtual auto get_topology() const -> vk::PrimitiveTopology = 0;
};
} // namespace le
