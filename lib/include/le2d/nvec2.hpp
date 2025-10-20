#pragma once
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>

namespace le {
inline constexpr auto right_v = glm::vec2{1.0f, 0.0f};
inline constexpr auto up_v = glm::vec2{0.0f, 1.0f};

/// \brief Normalized 2D vector.
class nvec2 : public glm::vec2 {
  public:
	explicit(false) nvec2(glm::vec2 xy = right_v);
	explicit(false) nvec2(float x, float y) : nvec2(glm::vec2{x, y}) {}

	[[nodiscard]] auto operator-() const -> nvec2 { return {InPlace{}, {-x, -y}}; }

	[[nodiscard]] static auto from_radians(float r) -> nvec2;
	[[nodiscard]] auto to_radians() const -> float;

	[[nodiscard]] auto rotated(float radians) const -> nvec2;
	void rotate(float radians);

  private:
	struct InPlace {};
	nvec2(InPlace /*d*/, glm::vec2 const xy) : glm::vec2(xy) {}
};
} // namespace le
