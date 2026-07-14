#include "demo/scene/scene.hpp"
#include <imgui.h>

namespace demo {
void Scene::tick(kvf::Seconds const /*dt*/) {
	ImGui::SetNextWindowSize({300.0f, 200.0f}, ImGuiCond_Once);
	ImGui::Begin("demo");
	ImGui::TextUnformatted("update your scene here");
	ImGui::End();
}

void Scene::render_frame() const {
	render_custom_passes();
	auto& renderer = m_context->begin_render(m_main_pass_clear);
	render_main_pass(renderer);
	renderer.end_render();
}
} // namespace demo
