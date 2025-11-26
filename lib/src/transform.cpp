#include "le2d/transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace le {
namespace {
struct Trs {
	glm::mat4 t;
	glm::mat4 r;
	glm::mat4 s;

	explicit Trs(glm::vec2 const position, float const rotation, glm::vec2 const scale)
		: t(glm::translate(Transform::identity_mat_v, glm::vec3{position, 0.0f})),
		  r(glm::rotate(Transform::identity_mat_v, rotation, glm::vec3{0.0f, 0.0f, 1.0f})), s(glm::scale(Transform::identity_mat_v, glm::vec3{scale, 1.0f})) {}
};
} // namespace

auto Transform::to_model() const -> glm::mat4 {
	auto const trs = Trs{position, orientation.to_radians(), scale};
	return trs.t * trs.r * trs.s;
}

auto Transform::to_view() const -> glm::mat4 {
	auto const trs = Trs{-position, -orientation.to_radians(), scale};
	return trs.s * trs.r * trs.t;
}

auto Transform::to_inverse_view() const -> glm::mat4 {
	if (scale == glm::vec2{0.0f}) { return {}; }
	auto const trs = Trs{position, orientation.to_radians(), 1.0f / scale};
	return trs.t * trs.r * trs.s;
}

auto Transform::accumulate(Transform const& a, Transform const& b) -> Transform {
	return Transform{
		.position = a.position + b.position,
		.orientation = nvec2::from_radians(a.orientation.to_radians() + b.orientation.to_radians()),
		.scale = a.scale * b.scale,
	};
}
} // namespace le
