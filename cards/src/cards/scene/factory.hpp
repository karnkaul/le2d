#pragma once
#include "cards/scene/scene.hpp"
#include <functional>
#include <unordered_map>

namespace cards::scene {
class Factory {
  public:
	explicit Factory(gsl::not_null<ICoordinator*> coordinator) : m_coordinator(coordinator) {}

	using Creator = std::move_only_function<std::unique_ptr<Scene>()>;

	void store_creator(std::string_view name, Creator creator);

	auto set_next_scene(std::string_view name) -> bool;
	[[nodiscard]] auto get_next_scene() -> std::unique_ptr<Scene>;

  private:
	gsl::not_null<ICoordinator*> m_coordinator;

	std::unordered_map<std::string_view, Creator> m_creators{};

	struct {
		klib::Ptr<Creator> creator{};
		std::string_view name{};
	} m_next{};
};
} // namespace cards::scene
