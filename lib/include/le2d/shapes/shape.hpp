#pragma once
#include <le2d/primitive.hpp>
#include <concepts>

namespace le {
template <typename Type>
concept ShapeT = requires(Type const& t) {
	{ t.get_primitive() } -> std::convertible_to<Primitive>;
};
} // namespace le
