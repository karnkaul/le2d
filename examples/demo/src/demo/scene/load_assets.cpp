#include "demo/scene/load_assets.hpp"
#include "klib/debug/assert.hpp"
#include "klib/string/fixed_string.hpp"
#include <imgui.h>
#include <filesystem>
#include <ranges>

namespace demo::scene {
namespace {
using namespace std::string_view_literals;

namespace fs = std::filesystem;

struct IndexedInspector {
	void inspect_uri(std::span<char> out) const {
		auto const uri_label = klib::FixedString{"##uri_{}", index};
		ImGui::SetNextItemWidth(300.0f);
		ImGui::InputText(uri_label.c_str(), out.data(), out.size() - 1);
	}

	void inspect_type(AssetType& out) const {
		auto const type_label = klib::FixedString{"##type_{}", index};
		ImGui::SetNextItemWidth(120.0f);
		auto const current = asset_type_name_map.to_name(out);
		if (!ImGui::BeginCombo(type_label.c_str(), current.data())) { return; }

		for (auto const [type, name] : asset_type_name_map.as_span()) {
			if (ImGui::Selectable(name.data(), type == out)) { out = type; }
		}
		ImGui::EndCombo();
	}

	[[nodiscard]] auto delete_entry() const -> bool {
		auto const x_label = klib::FixedString{"X##delete_{}", index};
		return ImGui::SmallButton(x_label.c_str());
	}

	int index{};
};

[[nodiscard]] auto get_asset_type(std::string_view const extension) -> std::optional<AssetType> {
	static constexpr auto images_v = std::array{
		".jpg",
		".png",
		".tga",
	};
	if (std::ranges::find(images_v, extension) != images_v.end()) { return AssetType::Texture; }

	static constexpr auto audio_v = std::array{
		".wav",
		".mp3",
		".flac",
	};
	if (std::ranges::find(audio_v, extension) != audio_v.end()) { return AssetType::AudioBuffer; }

	static constexpr auto fonts_v = std::array{
		".ttf",
		".otf",
	};
	if (std::ranges::find(fonts_v, extension) != fonts_v.end()) { return AssetType::Font; }

	return {};
}

void autopopulate(fs::path const& assets_root, std::vector<AssetInfo>& out) {
	for (auto const& it : fs::recursive_directory_iterator{assets_root}) {
		if (!it.is_regular_file()) { continue; }

		auto const type = get_asset_type(it.path().extension().string());
		if (!type) { continue; }

		auto const uri = fs::relative(it.path(), assets_root);
		out.push_back(AssetInfo::create(uri.generic_string(), *type));
	}
}
} // namespace

auto AssetInfo::create(std::string_view const uri, AssetType const type) -> AssetInfo {
	KLIB_ASSERT(uri.size() < max_uri_length_v);
	auto ret = AssetInfo{.type = type};
	if (!uri.empty()) { std::memcpy(ret.uri_buffer.data(), uri.data(), uri.size()); }
	return ret;
}

LoadAssets::LoadAssets(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader)
	: Scene(context, data_loader, "LoadAssets"), m_map(context) {
	create_loaders();
}

void LoadAssets::create_loaders() {
	m_manifest_loader = le::IManifestLoader::create(&get_asset_loader());

	m_ui_manifest = {
		AssetInfo::create("images/awesomeface.png", AssetType::Texture),
		AssetInfo::create("fonts/Vera.ttf", AssetType::Font),
		AssetInfo::create("audio/explode.wav", AssetType::AudioBuffer),
	};
}

void LoadAssets::tick(kvf::Seconds const /*dt*/) {
	ImGui::SetNextWindowSize({600.0f, 500.0f});
	ImGui::Begin("Load Assets");

	ImGui::SeparatorText("Loader");
	auto const status = klib::FixedString{"status: {}", le::manifest_loader_state_name_map.to_name(m_manifest_loader->get_state())};
	ImGui::TextUnformatted(status.c_str());
	inspect_manifest();

	using State = le::ManifestLoaderState;

	auto const state = m_manifest_loader->get_state();
	ImGui::BeginDisabled(state != State::Idle);
	if (ImGui::Button("Start")) { start_loading(); }
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(m_manifest_loader->get_state() != State::Loading);
	if (ImGui::Button("Cancel")) { m_manifest_loader->cancel(); }
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(m_manifest_loader->get_state() != State::Loaded);
	if (ImGui::Button("Transfer to Map")) { m_manifest_loader->transfer_assets(m_map); }
	ImGui::EndDisabled();

	ImGui::SeparatorText("Progress");
	auto const progress = m_manifest_loader->get_progress();
	ImGui::TextUnformatted(klib::FixedString{"total: {}", progress.total}.c_str());
	ImGui::TextUnformatted(klib::FixedString{"loaded: {}", progress.loaded}.c_str());
	ImGui::TextUnformatted(klib::FixedString{"failed: {}", progress.failed}.c_str());
	ImGui::ProgressBar(progress.normalized());

	ImGui::SeparatorText("Map");
	ImGui::TextUnformatted(klib::FixedString{"asset count: {}", m_map.asset_count()}.c_str());
	inspect_map();
	if (ImGui::Button("Clear")) { m_map.clear_assets(); }

	ImGui::End();
}

void LoadAssets::inspect_manifest() {
	if (!ImGui::TreeNode("Manifest")) { return; }
	for (auto [index, info] : std::views::enumerate(m_ui_manifest)) {
		auto const inspector = IndexedInspector{.index = int(index)};
		inspector.inspect_type(info.type);
		ImGui::SameLine();
		inspector.inspect_uri(info.uri_buffer);
		ImGui::SameLine();
		if (inspector.delete_entry()) {
			m_ui_manifest.erase(m_ui_manifest.begin() + index);
			break;
		}
	}
	if (ImGui::Button("add")) { m_ui_manifest.push_back(AssetInfo::create({}, AssetType::Texture)); }
	ImGui::SameLine();
	if (ImGui::Button("clear")) { m_ui_manifest.clear(); }
	ImGui::SameLine();
	if (ImGui::Button("autopopulate")) { autopopulate(get_file_data_loader().get_root_dir(), m_ui_manifest); }

	ImGui::TreePop();
}

void LoadAssets::inspect_map() const {
	if (!ImGui::TreeNode("Assets")) { return; }
	for (auto const asset_view : m_map.build_asset_views()) {
		auto const type = [&] {
			if (dynamic_cast<le::IFont*>(asset_view.asset.get())) { return AssetType::Font; }
			if (dynamic_cast<le::IAudioBuffer*>(asset_view.asset.get())) { return AssetType::AudioBuffer; }
			return AssetType::Texture;
		}();
		ImGui::TextUnformatted(klib::FixedString{"[{}] {}", asset_type_name_map.to_name(type), asset_view.uri->get_string()}.c_str());
	}
	ImGui::TreePop();
}

void LoadAssets::start_loading() {
	auto manifest = le::AssetManifest{};
	for (auto const& info : m_ui_manifest) {
		auto uri = le::Uri{info.uri_buffer.data()};
		if (uri.get_string().empty()) { continue; }

		switch (info.type) {
		case AssetType::Texture: manifest.add_entry<le::ITexture>(std::move(uri)); break;
		case AssetType::Font: manifest.add_entry<le::IFont>(std::move(uri)); break;
		case AssetType::AudioBuffer: manifest.add_entry<le::IAudioBuffer>(std::move(uri)); break;
		default: break;
		}
	}

	m_manifest_loader->start_loading(std::move(manifest));
}
} // namespace demo::scene
