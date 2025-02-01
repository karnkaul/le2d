#pragma once
#include <klib/polymorphic.hpp>
#include <scene/scene.hpp>
#include <functional>
#include <memory>

namespace hog::scene {
using SwitchFunc = std::move_only_function<std::unique_ptr<Scene>()>;

class ISwitcher : public klib::Polymorphic {
  public:
	virtual void switch_scene(SwitchFunc create_scene) = 0;
};
} // namespace hog::scene
