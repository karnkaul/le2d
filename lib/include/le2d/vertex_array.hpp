#pragma once
#include <le2d/vertex.hpp>
#include <cstdint>
#include <span>
#include <vector>

namespace le {
/// \brief Drawable vertex array.
struct VertexArray {
	std::vector<Vertex> vertices{};
	std::vector<std::uint32_t> indices{};

	void reserve(std::size_t vertex_count, std::size_t index_count);
	void clear();
	auto append(std::span<Vertex const> vertices, std::span<std::uint32_t const> indices) -> VertexArray&;
};
} // namespace le
