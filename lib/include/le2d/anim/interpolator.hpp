#pragma once
#include "le2d/transform.hpp"
#include <glm/common.hpp>
#include <cmath>

namespace le::anim {
template <typename Type>
struct Interpolator;

/// \brief Wrapper for Interpolator<Type>{}(a, b, t).
template <typename Type>
constexpr auto interpolate(Type const& a, Type const& b, float const t) {
	return Interpolator<Type>{}(a, b, t);
}

/// \brief Base class template for linear interpolation.
template <typename Type>
struct Interpolator {
	constexpr auto operator()(Type const& a, Type const& b, float const t) const { return std::lerp(a, b, t); }
};

/// \brief Interpolator specialization for glm::vec types.
template <typename Type, glm::length_t Length>
struct Interpolator<glm::vec<Length, Type>> {
	constexpr auto operator()(glm::vec<Length, Type> const a, glm::vec<Length, Type> const b, float const t) const { return glm::mix(a, b, t); }
};

/// \brief Interpolator specialization for Transform.
template <>
struct Interpolator<Transform> {
	auto operator()(Transform const& a, Transform const& b, float const t) const {
		return Transform{
			.position = interpolate(a.position, b.position, t),
			.orientation = nvec2::from_radians(interpolate(a.orientation.to_radians(), b.orientation.to_radians(), t)),
			.scale = interpolate(a.scale, b.scale, t),
		};
	}
};
} // namespace le::anim
