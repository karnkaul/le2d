#include <le2d/vertex_array.hpp>
#include <algorithm>

namespace le {
namespace {
void append_verts(VertexArray& out, std::span<Vertex const> vertices, std::span<std::uint32_t const> indices) {
	auto const index_offset = out.vertices.size();
	out.vertices.insert(out.vertices.end(), vertices.begin(), vertices.end());
	out.indices.reserve(out.indices.size() + indices.size());
	std::ranges::transform(indices, std::back_inserter(out.indices), [index_offset](std::uint32_t i) { return index_offset + i; });
}
} // namespace

auto VertexArray::append(std::span<Vertex const> vertices, std::span<std::uint32_t const> indices) -> VertexArray& {
	append_verts(*this, vertices, indices);
	return *this;
}
} // namespace le
