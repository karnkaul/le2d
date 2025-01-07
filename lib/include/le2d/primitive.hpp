#pragma once
#include <le2d/texture.hpp>
#include <le2d/vertex.hpp>
#include <cstdint>
#include <span>

namespace le {
struct Primitive {
	std::span<Vertex const> vertices{};
	std::span<std::uint32_t const> indices{};
	vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
	ITexture const* texture{};
};
} // namespace le
