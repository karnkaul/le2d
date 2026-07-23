#pragma once
#include "cards/catalog/catalog.hpp"
#include "cards/scene/factory.hpp"
#include "cards/scene/scene.hpp"
#include "le2d/context.hpp"
#include "le2d/file_data_loader.hpp"
#include "le2d/input/router.hpp"

namespace cards {
class App : public scene::ICoordinator {
  public:
	void run();

  private:
	[[nodiscard]] auto get_context() const -> le::Context& final { return *m_context; }
	[[nodiscard]] auto get_data_loader() const -> le::IDataLoader const& final { return m_data_loader; }
	[[nodiscard]] auto get_catalog() const -> Catalog const& final { return m_catalog; }
	[[nodiscard]] auto get_input_router() const -> le::input::Router& final { return *m_input_router; }
	[[nodiscard]] auto get_unprojector() const -> le::Unprojector const& final { return m_unprojector; }
	[[nodiscard]] auto get_random() const -> le::Random& final { return *m_random; }

	auto enqueue_scene(std::string_view name) -> bool final { return m_scene_factory->set_next_scene(name); }

	std::unique_ptr<le::Context> m_context{};
	le::FileDataLoader m_data_loader{};
	std::unique_ptr<le::input::Router> m_input_router{std::make_unique<le::input::Router>()};
	std::optional<le::AssetMap> m_asset_map{};
	std::unique_ptr<le::Random> m_random{std::make_unique<le::Random>()};

	Catalog m_catalog{};
	std::optional<scene::Factory> m_scene_factory{};
	std::unique_ptr<Scene> m_scene{};

	le::Unprojector m_unprojector{};

	le::Context::Waiter m_waiter{};
};
} // namespace cards
