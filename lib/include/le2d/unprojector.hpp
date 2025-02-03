#pragma once
#include <le2d/transform.hpp>
#include <le2d/vector_space.hpp>

namespace le {
class Unprojector {
  public:
	explicit Unprojector(Transform const& view, glm::vec2 const framebuffer_size)
		: m_inverse_view(view.to_inverse_view()), m_framebuffer_size(framebuffer_size) {}

	[[nodiscard]] auto framebuffer_size() const -> glm::vec2 { return m_framebuffer_size; }

	[[nodiscard]] auto to_framebuffer(ndc::vec2 const point) const -> glm::vec2 { return point.to_target(framebuffer_size()); }

	[[nodiscard]] auto unproject(ndc::vec2 const point) const -> glm::vec2 {
		auto const pos = to_framebuffer(point);
		return m_inverse_view * glm::vec4{pos, 0.0f, 1.0f};
	}

  private:
	glm::mat4 m_inverse_view{};
	glm::vec2 m_framebuffer_size{};
};
} // namespace le
