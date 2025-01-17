#pragma once
#include <le2d/vertex.hpp>
#include <cstdint>
#include <span>
#include <vector>

namespace le {
struct VertexArray {
	std::vector<Vertex> vertices{};
	std::vector<std::uint32_t> indices{};

	auto append(std::span<Vertex const> vertices, std::span<std::uint32_t const> indices) -> VertexArray&;
};
} // namespace le
