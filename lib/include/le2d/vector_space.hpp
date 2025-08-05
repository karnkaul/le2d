#pragma once
#include <kvf/is_positive.hpp>
#include <kvf/util.hpp>

namespace le {
namespace uv {
struct vec2;
} // namespace uv

namespace ndc {
struct vec2 : glm::vec2 {
	using glm::vec2::vec2;

	explicit(false) constexpr vec2(glm::vec2 const in) : glm::vec2(in) {}

	[[nodiscard]] constexpr auto to_target(glm::vec2 const target_size) const -> glm::vec2 { return *this * target_size; }

	[[nodiscard]] constexpr auto to_uv() const -> uv::vec2;
};
} // namespace ndc

namespace uv {
struct vec2 : glm::vec2 {
	using glm::vec2::vec2;

	explicit(false) constexpr vec2(glm::vec2 const in) : glm::vec2(in) {}

	[[nodiscard]] constexpr auto to_target(glm::vec2 const target_size) const -> glm::vec2 { return *this * target_size; }
	[[nodiscard]] constexpr auto to_ndc() const -> ndc::vec2;
};
} // namespace uv

namespace window {
template <typename Type>
struct tvec2 : glm::tvec2<Type> {
	using glm::tvec2<Type>::tvec2;

	[[nodiscard]] constexpr auto to_ndc(glm::vec2 const window_size) const -> ndc::vec2 {
		if (!kvf::is_positive(window_size)) { return {}; }
		return uv::vec2{glm::vec2(*this) / window_size}.to_ndc();
	}
};

using ivec2 = tvec2<int>;
using vec2 = tvec2<float>;
} // namespace window

constexpr auto ndc::vec2::to_uv() const -> uv::vec2 { return kvf::util::ndc_to_uv(*this); }

constexpr auto uv::vec2::to_ndc() const -> ndc::vec2 { return kvf::util::uv_to_ndc(*this); }

[[nodiscard]] constexpr auto uv_to_world(kvf::UvRect const& uv, glm::vec2 const target_size) -> kvf::Rect<> { return target_size * kvf::util::uv_to_ndc(uv); }

[[nodiscard]] constexpr auto world_to_uv(kvf::Rect<> const& rect) -> kvf::UvRect {
	auto const size = rect.size();
	if (!kvf::is_positive(size)) { return {}; }
	return kvf::util::ndc_to_uv(rect / size);
}
} // namespace le
