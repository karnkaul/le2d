#include "demo/scene/input_actions.hpp"
#include <imgui.h>
#include <algorithm>

namespace demo::scene {
Player::Player() {
	auto& vertices = triangle.geometry.vertices;
	vertices[0].position = {-80.0f, 50.0f};
	vertices[1].position = {-80.0f, -50.0f};
	vertices[2].position = {80.0f, 0.0f};
}

void Player::draw(le::IRenderer& renderer) const { triangle.draw(renderer); }

void Player::inspect() { ImGui::DragFloat("speed", &speed); }

void IController::bind_actions(le::input::ActionMapping& out_mapping) {
	out_mapping.bind_action(&get_movement_action(), [this](le::input::action::Value const& v) { m_delta_xy = v.get<glm::vec2>(); });
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
	m_router.push_mapping(m_mapping);

	create_controllers();
	set_active_controller(m_controllers.storage.front().get());
}

void InputActions::create_controllers() {
	m_controllers.storage.push_back(std::make_unique<KeyboardController>());
	m_controllers.storage.push_back(std::make_unique<GamepadController>());
	for (auto const& controller : m_controllers.storage) { controller->bind_actions(*m_mapping); }
}

void InputActions::tick(kvf::Seconds const dt) {
	m_router.dispatch(get_context().event_queue());

	update_active_controller();

	for (auto const& controller : m_controllers.storage) { controller->tick(dt); }

	inspect();
}

void InputActions::set_active_controller(gsl::not_null<IController*> controller) {
	if (m_controllers.active) { m_controllers.active->unpossess(); }
	m_controllers.active = controller;
	m_controllers.active->possess(&m_player);
}

void InputActions::update_active_controller() {
	auto const last_used_device = m_router.last_used_device();
	if (!last_used_device || (*last_used_device != KeyboardController::device_v && *last_used_device != GamepadController::device_v)) { return; }

	auto const current_device = *last_used_device;
	if (m_controllers.current_device == current_device) { return; }

	auto const pred = [current_device](auto const& controller) { return controller->get_device() == current_device; };
	auto const it = std::ranges::find_if(m_controllers.storage, pred);
	if (it == m_controllers.storage.end()) { return; }

	m_controllers.current_device = current_device;
	set_active_controller(it->get());
}

void InputActions::render_main_pass(le::IRenderer& renderer) const { m_player.draw(renderer); }

void InputActions::inspect() {
	ImGui::SetNextWindowSize({400.0f, 300.0f});
	ImGui::Begin("Editor");
	m_player.inspect();
	ImGui::End();
}
} // namespace demo::scene
