#pragma once
#include <klib/polymorphic.hpp>
#include <le2d/service_locator.hpp>
#include <scene/scene.hpp>
#include <functional>
#include <memory>

namespace hog::scene {
class ISwitcher : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_services() const -> le::ServiceLocator const& = 0;

	template <std::derived_from<Scene> SceneT, typename... Args>
		requires(std::constructible_from<SceneT, le::ServiceLocator const*, Args...>)
	void switch_scene(Args... args) {
		auto const create = [s = &get_services(), ... args = std::move(args)] { return std::make_unique<SceneT>(s, std::move(args)...); };
		enqueue_switch(create);
	}

  protected:
	using SwitchFunc = std::move_only_function<std::unique_ptr<Scene>()>;

	virtual void enqueue_switch(SwitchFunc create_scene) = 0;
};
} // namespace hog::scene
