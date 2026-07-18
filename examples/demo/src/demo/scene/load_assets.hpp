#pragma once
#include "demo/scene/scene.hpp"
#include "le2d/asset/manifest_loader.hpp"
#include <array>

namespace demo::scene {
enum class AssetType : std::int8_t { Texture, Font, AudioBuffer };
inline static auto const asset_type_name_map = klib::EnumNameMap<AssetType>{
	{AssetType::Texture, "Texture"},
	{AssetType::Font, "Font"},
	{AssetType::AudioBuffer, "AudioBuffer"},
};

struct AssetInfo {
	static constexpr auto max_uri_length_v{256uz};

	[[nodiscard]] static auto create(std::string_view uri, AssetType type) -> AssetInfo;

	std::array<char, max_uri_length_v> uri_buffer{};
	AssetType type{};
};

class LoadAssets : public Scene {
  public:
	static constexpr std::string_view name_v{"Load Assets"};

	explicit LoadAssets(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader);

  private:
	void tick(kvf::Seconds dt) final;

	void create_loaders();

	void inspect_manifest();
	void inspect_map() const;

	void start_loading();

	std::unique_ptr<le::IManifestLoader> m_manifest_loader{};
	le::AssetMap m_map;

	std::vector<AssetInfo> m_ui_manifest{};
};
} // namespace demo::scene
