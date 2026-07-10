#pragma once
#include "klib/base_types.hpp"
#include "klib/ptr.hpp"
#include "le2d/primitive.hpp"
#include "le2d/vertex.hpp"
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace le {
class ITextureBase;

/// \brief Interface for drawable geometry.
class IGeometry : public klib::Polymorphic {
  public:
	static constexpr auto default_length_v{200.0f};

	[[nodiscard]] virtual auto get_vertices() const -> std::span<Vertex const> = 0;
	[[nodiscard]] virtual auto get_indices() const -> std::span<std::uint32_t const> = 0;
	[[nodiscard]] virtual auto get_topology() const -> vk::PrimitiveTopology = 0;

	[[nodiscard]] auto to_primitive(klib::Ptr<ITextureBase const> texture) const -> Primitive {
		return Primitive{
			.vertices = get_vertices(),
			.indices = get_indices(),
			.topology = get_topology(),
			.texture = texture,
		};
	}
};
} // namespace le
