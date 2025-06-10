#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace le {
/// \brief Drawable vertex.
struct Vertex {
	glm::vec2 position{};
	glm::vec4 color{1.0f};
	glm::vec2 uv{};
};
} // namespace le
