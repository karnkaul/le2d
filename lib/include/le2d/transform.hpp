#pragma once
#include <glm/mat4x4.hpp>

namespace le {
struct Transform {
	static constexpr auto identity_mat_v = glm::mat4{1.0f};

	[[nodiscard]] auto to_model() const -> glm::mat4;
	[[nodiscard]] auto to_view() const -> glm::mat4;

	glm::vec2 position{};
	float orientation{};
	glm::vec2 scale{1.0f};
};
} // namespace le
