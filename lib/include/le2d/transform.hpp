#pragma once
#include <glm/mat4x4.hpp>

namespace le {
struct Transform {
	static constexpr auto identity_mat_v = glm::mat4{1.0f};

	[[nodiscard]] auto to_model() const -> glm::mat4;
	[[nodiscard]] auto to_view() const -> glm::mat4;
	[[nodiscard]] auto to_inverse_view() const -> glm::mat4;

	[[nodiscard]] static constexpr auto accumulate(Transform const& a, Transform const& b) -> Transform {
		return Transform{
			.position = a.position + b.position,
			.orientation = a.orientation + b.orientation,
			.scale = a.scale * b.scale,
		};
	}

	glm::vec2 position{};
	float orientation{};
	glm::vec2 scale{1.0f};
};
} // namespace le
