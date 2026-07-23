#pragma once
#include "cards/catalog/catalog.hpp"
#include "le2d/asset/asset_manifest.hpp"
#include "le2d/asset/asset_map.hpp"

namespace cards {
class Catalog::Resources {
  public:
	explicit Resources(Catalog& catalog) : m_catalog(catalog) {}

	void populate_manifest(le::AssetManifest& out) const;
	void assign_assets(le::AssetMap const& asset_map);

  private:
	Catalog& m_catalog;
};
} // namespace cards
