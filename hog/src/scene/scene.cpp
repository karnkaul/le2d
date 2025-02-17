#include <le2d/context.hpp>
#include <le2d/input/dispatch.hpp>
#include <le2d/service_locator.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
Scene::Scene(gsl::not_null<le::ServiceLocator const*> services) : m_services(services) { m_services->get<le::input::Dispatch>().attach(this); }

auto Scene::get_unprojector(le::Transform const& render_view) const -> le::Unprojector {
	auto const framebuffer_size = m_services->get<le::Context>().framebuffer_size();
	return le::Unprojector{render_view, framebuffer_size};
}
} // namespace hog::scene
