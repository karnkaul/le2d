#include "le2d/nvec2.hpp"
#include <glm/mat2x2.hpp>
#include <klib/assert.hpp>

namespace le {
auto nvec2::normal_or(glm::vec2 const xy, glm::vec2 const fallback) -> nvec2 {
	auto const length = glm::length2(xy);
	if (length == 0.0f) { return fallback; }
	return nvec2{InPlace{}, xy / glm::sqrt(length)};
}

nvec2::nvec2(glm::vec2 const xy) : glm::vec2(xy) {
	auto const length = glm::length(static_cast<glm::vec2>(*this));
	KLIB_ASSERT(length > 0.0f);
	x /= length;
	y /= length;
}

auto nvec2::from_radians(float const r) -> nvec2 { return {InPlace{}, {glm::cos(r), glm::sin(r)}}; }

auto nvec2::to_radians() const -> float {
	auto const dot = glm::dot(*this, right_v);
	auto const rad = glm::acos(dot);
	return y < 0.0f ? -rad : rad;
}

auto nvec2::rotated(float const radians) const -> nvec2 {
	auto ret = *this;
	ret.rotate(radians);
	return ret;
}

void nvec2::rotate(float const radians) {
	auto const c = glm::cos(radians);
	auto const s = glm::sin(radians);
	auto const mat = glm::mat2{
		glm::vec2{c, -s},
		glm::vec2{s, c},
	};
	*this = glm::vec2{*this} * mat;
}
} // namespace le
