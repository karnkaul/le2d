#pragma once
#include <glm/gtx/norm.hpp>
#include <glm/mat4x4.hpp>
#include <le2d/nvec2.hpp>

namespace le {
/// \brief 2D transformation.
struct Transform {
	static constexpr auto identity_mat_v = glm::mat4{1.0f};

	/// \returns Model matrix.
	[[nodiscard]] auto to_model() const -> glm::mat4;
	/// \returns View matrix.
	[[nodiscard]] auto to_view() const -> glm::mat4;
	/// \returns Inverse of view matrix.
	[[nodiscard]] auto to_inverse_view() const -> glm::mat4;

	/// \returns Transform with positions and orientations added and scales multiplied together.
	[[nodiscard]] static auto accumulate(Transform const& a, Transform const& b) -> Transform;

	glm::vec2 position{};
	nvec2 orientation{right_v};
	glm::vec2 scale{1.0f};
};
} // namespace le
