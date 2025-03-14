#pragma once
#include <glm/vec2.hpp>
#include <kvf/is_positive.hpp>
#include <kvf/util.hpp>

namespace le {
namespace ndc {
struct vec2 : glm::vec2 {
	using glm::vec2::vec2;

	[[nodiscard]] constexpr auto to_target(glm::vec2 const target_size) const -> glm::vec2 { return *this * target_size; }
};
} // namespace ndc

namespace window {
template <typename Type>
struct tvec2 : glm::tvec2<Type> {
	using glm::tvec2<Type>::tvec2;

	[[nodiscard]] constexpr auto to_ndc(glm::vec2 const window_size) const -> ndc::vec2 {
		if (!kvf::is_positive(window_size)) { return {}; }
		auto const ret = kvf::util::uv_to_ndc(*this / window_size);
		return {ret.x, ret.y};
	}
};

using ivec2 = tvec2<int>;
using vec2 = tvec2<float>;
} // namespace window
} // namespace le
