#pragma once
#include "demo/scene/scene.hpp"
#include "le2d/drawable/shape.hpp"
#include "le2d/input/action_mapping.hpp"
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
	[[nodiscard]] virtual auto get_device() const -> le::input::Device = 0;

	void bind_actions(le::input::ActionMapping& out_mapping);

	void tick(kvf::Seconds dt);

	void possess(gsl::not_null<Player*> player);
	void unpossess();

  protected:
	virtual auto get_movement_action() -> le::input::IAction& = 0;

	klib::Ptr<Player> m_player{};

	glm::vec2 m_delta_xy{};
};

class KeyboardController : public IController {
  public:
	static constexpr auto device_v{le::input::Device::Keyboard};

	[[nodiscard]] auto get_device() const -> le::input::Device final { return device_v; }

  private:
	auto get_movement_action() -> le::input::IAction& final { return m_actions.movement; }

	struct {
		le::input::action::KeyAxis2D movement{{GLFW_KEY_A, GLFW_KEY_D}, {GLFW_KEY_S, GLFW_KEY_W}};
	} m_actions{};
};

class GamepadController : public IController {
  public:
	static constexpr auto device_v{le::input::Device::Gamepad};

	[[nodiscard]] auto get_device() const -> le::input::Device final { return device_v; }

  private:
	auto get_movement_action() -> le::input::IAction& final { return m_actions.movement; }

	struct {
		le::input::action::GamepadAxis2D movement{GLFW_GAMEPAD_AXIS_LEFT_X, GLFW_GAMEPAD_AXIS_LEFT_Y};
	} m_actions{};
};

class InputActions : public Scene {
  public:
	static constexpr std::string_view name_v{"InputActions"};

	explicit InputActions(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader);

  private:
	void create_controllers();

	void tick(kvf::Seconds dt) final;
	void render_main_pass(le::IRenderer& renderer) const final;

	void set_active_controller(gsl::not_null<IController*> controller);
	void update_active_controller();

	void inspect();

	le::input::Router m_router{};
	std::shared_ptr<le::input::ActionMapping> m_mapping{std::make_shared<le::input::ActionMapping>()};

	struct {
		std::vector<std::unique_ptr<IController>> storage{};
		klib::Ptr<IController> active{};
		le::input::Device current_device{le::input::Device::Keyboard};
	} m_controllers{};

	Player m_player{};
};
} // namespace demo::scene
