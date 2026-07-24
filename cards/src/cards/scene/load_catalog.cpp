#include "cards/scene/load_catalog.hpp"
#include "cards/catalog/catalog_parser.hpp"
#include "cards/catalog/catalog_resources.hpp"
#include "djson/json.hpp"
#include "klib/string/fixed_string.hpp"
#include <imgui.h>

namespace cards::scene {
LoadCatalog::LoadCatalog(gsl::not_null<Catalog*> catalog, gsl::not_null<le::AssetMap*> asset_map, std::string_view const next_scene)
	: m_catalog(catalog), m_asset_map(asset_map), m_next_scene(next_scene) {}

void LoadCatalog::initialize() {
	auto const& coordinator = get_coordinator();

	dj::Json const json = coordinator.get_data_loader().load_json("catalog.jsonc");
	if (!json) { throw std::runtime_error{"Failed to load Catalog"}; }

	parse_catalog(*m_catalog, json);
	auto manifest = le::AssetManifest{};
	Catalog::Resources{*m_catalog}.populate_manifest(manifest);

	m_asset_loader = coordinator.get_context().create_asset_loader(&coordinator.get_data_loader());
	m_manifest_loader = le::IManifestLoader::create(&m_asset_loader);
	m_manifest_loader->start_loading(std::move(manifest));
}

void LoadCatalog::tick([[maybe_unused]] kvf::Seconds dt) {
	check_loaded();
	tick_loading();
}

void LoadCatalog::check_loaded() {
	if (m_manifest_loader->get_state() == le::ManifestLoaderState::Loading) { return; }

	m_manifest_loader->transfer_assets(*m_asset_map);
	Catalog::Resources{*m_catalog}.assign_assets(*m_asset_map);

	get_coordinator().enqueue_scene(m_next_scene);
}

void LoadCatalog::tick_loading() {
	static constexpr auto title_v = klib::CString{"Loading Catalog"};
	if (!ImGui::IsPopupOpen(title_v.c_str())) { ImGui::OpenPopup(title_v.c_str()); }

	ImGui::SetNextWindowSize({200.0f, 100.0f});
	auto const center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
	if (!ImGui::BeginPopupModal(title_v.c_str())) { return; }

	auto const progress = m_manifest_loader->get_progress();
	ImGui::TextUnformatted(klib::FixedString{"progress: {} / {}", progress.completed(), progress.total}.c_str());
	ImGui::ProgressBar(progress.normalized());

	ImGui::EndPopup();
}
} // namespace cards::scene
