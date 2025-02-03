#include <le2d/context.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
auto Scene::get_unprojector(le::Transform const& render_view) const -> le::Unprojector {
	auto const framebuffer_size = m_services->get<le::Context>().framebuffer_size();
	return le::Unprojector{render_view, framebuffer_size};
}
} // namespace hog::scene
