#include <glm/gtc/matrix_transform.hpp>
#include <le2d/transform.hpp>

namespace le {
namespace {
struct Trs {
	glm::mat4 t;
	glm::mat4 r;
	glm::mat4 s;

	explicit Trs(glm::vec2 const position, float const orientation, glm::vec2 const scale)
		: t(glm::translate(Transform::identity_mat_v, glm::vec3{position, 0.0f})),
		  r(glm::rotate(Transform::identity_mat_v, glm::radians(orientation), glm::vec3{0.0f, 0.0f, 1.0f})),
		  s(glm::scale(Transform::identity_mat_v, glm::vec3{scale, 1.0f})) {}
};
} // namespace

auto Transform::to_model() const -> glm::mat4 {
	auto const trs = Trs{position, orientation, scale};
	return trs.t * trs.r * trs.s;
}

auto Transform::to_view() const -> glm::mat4 {
	auto const trs = Trs{-position, -orientation, scale};
	return trs.s * trs.r * trs.t;
}

auto Transform::to_inverse_view() const -> glm::mat4 {
	if (scale == glm::vec2{0.0f}) { return {}; }
	auto const trs = Trs{position, orientation, 1.0f / scale};
	return trs.t * trs.r * trs.s;
}
} // namespace le
