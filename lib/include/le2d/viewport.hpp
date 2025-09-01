#pragma once
#include <kvf/rect.hpp>
#include <variant>

namespace le {
namespace viewport {
struct Dynamic {
	kvf::UvRect n_rect{kvf::uv_rect_v};
};

struct Letterbox {
	[[nodiscard]] auto fill_target_space(glm::vec2 target_size) const -> glm::vec2;
	[[nodiscard]] auto unproject_target_space(glm::vec2 target_size) const -> glm::vec2;

	glm::vec2 world_size{800.0f, 600.0f};
};
} // namespace viewport

using Viewport = std::variant<viewport::Dynamic, viewport::Letterbox>;
} // namespace le
