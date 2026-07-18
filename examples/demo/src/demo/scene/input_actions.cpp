#include "demo/scene/input_actions.hpp"
#include "le2d/input/action_mapping.hpp"
#include <imgui.h>

namespace demo::scene {
Player::Player() {
	auto& vertices = triangle.geometry.vertices;
	vertices[0].position = {-40.0f, 30.0f};
	vertices[1].position = {-40.0f, -30.0f};
	vertices[2].position = {40.0f, 0.0f};
}

void Player::draw(le::IRenderer& renderer) const { triangle.draw(renderer); }

void Player::inspect() { ImGui::DragFloat("speed", &speed); }

void IController::initialize() {
	auto mapping = std::make_shared<le::input::ActionMapping>();
	mapping->bind_action(&get_movement_action(), [this](le::input::action::Value const& value) { m_delta_xy = value.get<glm::vec2>(); });
	m_mapping = std::move(mapping);
}

void IController::tick(kvf::Seconds const dt) {
	if (!m_player) { return; }

	if (glm::length2(m_delta_xy) > 0.0f) {
		auto const dxy = m_player->speed * m_delta_xy * dt.count();
		m_player->triangle.instance.transform.position += dxy;
		m_player->triangle.instance.transform.orientation = dxy;
	}
}

void IController::possess(gsl::not_null<Player*> player) { m_player = player; }

void IController::unpossess() {
	m_player = nullptr;
	m_delta_xy = {};
}

InputActions::InputActions(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader) : Scene(context, data_loader, name_v) {
	create_controllers();
}

void InputActions::create_controllers() {
	m_controllers.push_back(std::make_unique<KeyboardController>());
	m_controllers.push_back(std::make_unique<GamepadController>());
	for (auto const& controller : m_controllers) {
		controller->initialize();
		m_router.push_mapping(controller->get_mapping());
		controller->possess(&m_player);
	}
}

void InputActions::tick(kvf::Seconds const dt) {
	m_router.dispatch(get_context().event_queue());

	for (auto const& controller : m_controllers) { controller->tick(dt); }

	inspect();
}

void InputActions::render_main_pass(le::IRenderer& renderer) const { m_player.draw(renderer); }

void InputActions::inspect() {
	ImGui::SetNextWindowSize({400.0f, 300.0f});
	ImGui::Begin("Editor");
	m_player.inspect();
	ImGui::End();
}
} // namespace demo::scene
