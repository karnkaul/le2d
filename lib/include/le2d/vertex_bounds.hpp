#pragma once
#include "kvf/rect.hpp"
#include "le2d/vertex.hpp"
#include <glm/mat4x4.hpp>
#include <span>

namespace le {
[[nodiscard]] auto vertex_bounds(std::span<Vertex const> vertices, glm::mat4 const& model) -> kvf::Rect<>;
} // namespace le
