#include <le2d/context.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
auto Scene::unproject(le::Transform const& render_view, le::ndc::vec2 const point) const -> glm::vec2 {
	auto const pos = point.to_target(m_services->get<le::Context>().framebuffer_size());
	return render_view.to_inverse_view() * glm::vec4{pos, 0.0f, 1.0f};
}
} // namespace hog::scene
