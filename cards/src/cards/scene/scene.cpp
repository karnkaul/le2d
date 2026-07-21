#include "cards/scene/scene.hpp"
#include "cards/world_space.hpp"

namespace cards {
void Scene::initialize_scene(gsl::not_null<scene::ICoordinator*> coordinator) {
	m_coordinator = coordinator;
	initialize();
}

void Scene::tick_frame(kvf::Seconds const dt) { tick(dt); }

void Scene::render_frame() {
	auto& renderer = m_coordinator->get_context().begin_render(clear_color);
	renderer.set_viewport(le::viewport::Letterbox{.world_size = world_space_v});
	render(renderer);
}
} // namespace cards
