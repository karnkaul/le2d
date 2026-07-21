#pragma once
#include "cards/services.hpp"

namespace cards::scene {
class ICoordinator : public IServices {
  public:
	virtual auto enqueue_scene(std::string_view name) -> bool = 0;
};
} // namespace cards::scene
