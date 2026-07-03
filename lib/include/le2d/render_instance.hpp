#pragma once
#include "kvf/color.hpp"
#include "le2d/transform.hpp"

namespace le {
/// \brief Instance data for instanced rendering.
struct RenderInstance {
	struct Std430 {
		glm::mat4 transform;
		glm::vec4 tint;
	};

	[[nodiscard]] auto to_std430(bool const linearize_tint = true) const -> Std430 {
		return Std430{
			.transform = transform.to_model(),
			.tint = linearize_tint ? tint.to_linear() : tint.to_vec4(),
		};
	}

	Transform transform{};
	kvf::Color tint{kvf::white_v};
};
} // namespace le
