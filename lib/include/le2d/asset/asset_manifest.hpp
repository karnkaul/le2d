#pragma once
#include "le2d/asset/asset.hpp"
#include "le2d/uri.hpp"
#include <typeindex>
#include <vector>

namespace le {
/// \brief Encapsulation of concrete IAsset type and its Uri.
class ManifestEntry {
  public:
	template <std::derived_from<IAsset> AssetTypeT>
	[[nodiscard]] static auto create(Uri uri) -> ManifestEntry {
		return ManifestEntry{typeid(AssetTypeT), std::move(uri)};
	}

	[[nodiscard]] auto get_type() const -> std::type_index { return m_type; }

	Uri uri{};

  private:
	explicit ManifestEntry(std::type_index const type, Uri uri) : uri(std::move(uri)), m_type(type) {}

	std::type_index m_type;
};

/// \brief List of ManifestEntry.
struct AssetManifest {
	using Entry = ManifestEntry;

	template <std::derived_from<IAsset> AssetTypeT>
	void add_entry(Uri uri) {
		if (uri.get_string().empty()) { return; }
		entries.push_back(Entry::create<AssetTypeT>(std::move(uri)));
	}

	std::vector<Entry> entries{};
};
} // namespace le
