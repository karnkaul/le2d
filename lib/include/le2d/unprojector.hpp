#pragma once
#include <le2d/transform.hpp>
#include <le2d/vector_space.hpp>
#include <le2d/viewport.hpp>

namespace le {
class Unprojector {
  public:
	Unprojector() = default;

	explicit Unprojector(Transform const& view, glm::vec2 const target_size) : m_inverse_view(view.to_inverse_view()), m_target_size(target_size) {}

	explicit Unprojector(Viewport const& viewport, Transform const& view, glm::vec2 framebuffer_size);

	[[nodiscard]] auto target_size() const -> glm::vec2 { return m_target_size; }

	[[nodiscard]] auto to_target(ndc::vec2 const point) const -> glm::vec2 { return point.to_target(target_size()); }

	[[nodiscard]] auto unproject(ndc::vec2 const point) const -> glm::vec2 { return unproject(to_target(point)); }

	[[nodiscard]] auto unproject(glm::vec2 const point) const -> glm::vec2 { return m_inverse_view * glm::vec4{point, 0.0f, 1.0f}; }

  private:
	glm::mat4 m_inverse_view{1.0f};
	glm::vec2 m_target_size{};
};
} // namespace le
