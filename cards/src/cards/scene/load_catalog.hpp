#pragma once
#include "cards/scene/scene.hpp"
#include "le2d/asset/asset_map.hpp"
#include "le2d/asset/manifest_loader.hpp"

namespace cards::scene {
class LoadCatalog : public Scene {
  public:
	explicit LoadCatalog(gsl::not_null<Catalog*> catalog, gsl::not_null<le::AssetMap*> asset_map, std::string_view next_scene);

  private:
	void initialize() final;
	void tick(kvf::Seconds dt) final;

	void check_loaded();
	void tick_loading();

	gsl::not_null<Catalog*> m_catalog;
	gsl::not_null<le::AssetMap*> m_asset_map;
	std::string_view m_next_scene;

	le::AssetLoader m_asset_loader{};
	std::unique_ptr<le::IManifestLoader> m_manifest_loader{};
};
} // namespace cards::scene
