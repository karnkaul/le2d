#pragma once
#include <glm/common.hpp>
#include <le2d/transform.hpp>
#include <cmath>

namespace le::anim {
template <typename Type>
struct Interpolator;

template <typename Type>
constexpr auto interpolate(Type const& a, Type const& b, float const t) {
	return Interpolator<Type>{}(a, b, t);
}

template <typename Type>
struct Interpolator {
	constexpr auto operator()(Type const& a, Type const& b, float const t) const { return std::lerp(a, b, t); }
};

template <typename Type, glm::length_t Length>
struct Interpolator<glm::vec<Length, Type>> {
	constexpr auto operator()(glm::vec<Length, Type> const a, glm::vec<Length, Type> const b, float const t) const { return glm::mix(a, b, t); }
};

template <>
struct Interpolator<Transform> {
	constexpr auto operator()(Transform const& a, Transform const& b, float const t) const {
		return Transform{
			.position = interpolate(a.position, b.position, t),
			.orientation = interpolate(a.orientation, b.orientation, t),
			.scale = interpolate(a.scale, b.scale, t),
		};
	}
};
} // namespace le::anim
