#pragma once
#include "demo/scene/scene.hpp"
#include "le2d/drawable/shape.hpp"
#include "le2d/input/action.hpp"
#include "le2d/input/router.hpp"

namespace demo::scene {
class Player {
  public:
	explicit Player();

	void draw(le::IRenderer& renderer) const;

	void inspect();

	le::drawable::Triangle triangle{};
	float speed{1000.0f};
};

class IController : public klib::Polymorphic {
  public:
	void initialize();
	[[nodiscard]] auto get_mapping() const -> std::shared_ptr<le::input::IMapping> const& { return m_mapping; }

	void tick(kvf::Seconds dt);

	void possess(gsl::not_null<Player*> player);
	void unpossess();

  protected:
	virtual auto get_movement_action() -> le::input::IAction& = 0;

	klib::Ptr<Player> m_player{};
	std::shared_ptr<le::input::IMapping> m_mapping{};

	glm::vec2 m_delta_xy{};
};

class KeyboardController : public IController {
	auto get_movement_action() -> le::input::IAction& final { return m_actions.movement; }

	struct {
		le::input::action::KeyAxis2D movement{{GLFW_KEY_A, GLFW_KEY_D}, {GLFW_KEY_S, GLFW_KEY_W}};
	} m_actions{};
};

class GamepadController : public IController {
	auto get_movement_action() -> le::input::IAction& final { return m_actions.movement; }

	struct {
		le::input::action::GamepadAxis2D movement{GLFW_GAMEPAD_AXIS_LEFT_X, GLFW_GAMEPAD_AXIS_LEFT_Y};
	} m_actions{};
};

class InputActions : public Scene {
  public:
	static constexpr std::string_view name_v{"Input Actions"};

	explicit InputActions(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader);

  private:
	void create_controllers();

	void tick(kvf::Seconds dt) final;
	void render_main_pass(le::IRenderer& renderer) const final;

	void inspect();

	le::input::Router m_router{};

	std::vector<std::unique_ptr<IController>> m_controllers{};

	Player m_player{};
};
} // namespace demo::scene
