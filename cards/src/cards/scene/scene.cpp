#include "cards/scene/scene.hpp"
#include "cards/game/world_space.hpp"

namespace cards {
void Scene::initialize_scene(gsl::not_null<scene::ICoordinator*> coordinator) {
	m_coordinator = coordinator;
	initialize();
}

void Scene::tick_frame(kvf::Seconds const dt) { tick(dt); }

void Scene::render_frame(le::IRenderer& renderer) {
	renderer.set_viewport(le::viewport::Letterbox{.world_size = world_space_v});
	render(renderer);
}
} // namespace cards
