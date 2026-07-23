#pragma once
#include "cards/catalog/catalog.hpp"
#include "cards/console.hpp"
#include "cards/scene/scene.hpp"

namespace cards::scene {
class CreateTerminal : public Scene {
  public:
	explicit CreateTerminal(gsl::not_null<Console*> console, gsl::not_null<Catalog*> catalog, std::string_view next_scene);

  private:
	void initialize() final;

	void tick(kvf::Seconds dt) final;

	gsl::not_null<Console*> m_console;
	gsl::not_null<Catalog*> m_catalog;
	std::string_view m_next_scene;
};
} // namespace cards::scene
