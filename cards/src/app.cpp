#include <imgui.h>
#include <app.hpp>
#include <klib/fixed_string.hpp>
#include <le2d/asset_loader.hpp>
#include <le2d/util.hpp>
#include <ranges>

namespace cards {
namespace {
constexpr auto context_ci = le::Context::CreateInfo{
	.window = le::WindowInfo{.size = {1280, 720}, .title = "cards"},
};
} // namespace

App::App() : m_context(context_ci) {}

void App::run() {
	auto const assets_dir = le::FileDataLoader::upfind("assets", le::util::exe_path());
	m_data_loader.set_root_dir(assets_dir);
	m_blocker = m_context.get_render_window().get_render_device().get_device();

	auto deck_sheet = le::AssetLoader{&m_data_loader, &m_context}.load_tile_sheet("sheets/deck.json");
	m_deck_sheet.emplace(std::move(deck_sheet));

	m_drawer.deck_sheet = &*m_deck_sheet;

	while (m_context.is_running()) {
		m_context.next_frame();
		tick(m_context.get_frame_stats().total_dt);
		if (auto renderer = m_context.begin_render()) {
			render(renderer);
			renderer.end_render();
		}
		m_context.present();
	}
}

void App::tick([[maybe_unused]] kvf::Seconds const dt) {
	if (ImGui::Begin("Debug")) {
		auto const suit_str = suit_str_v[m_card.suit];
		ImGui::SetNextItemWidth(50.0f);
		if (ImGui::BeginCombo("suit", suit_str.data())) {
			for (auto const [suit, str] : std::views::enumerate(suit_str_v)) {
				if (ImGui::Selectable(str.data(), Suit(suit) == m_card.suit)) { m_card.suit = Suit(suit); }
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		auto const rank_str = rank_str_v[m_card.rank];
		ImGui::SetNextItemWidth(50.0f);
		if (ImGui::BeginCombo("rank", rank_str.data())) {
			for (auto const [rank, str] : std::views::enumerate(rank_str_v)) {
				auto flags = int{};
				if (Rank(rank) == Rank::None) { flags |= ImGuiSelectableFlags_Disabled; }
				if (ImGui::Selectable(str.data(), Rank(rank) == m_card.rank, flags)) { m_card.rank = Rank(rank); }
			}
			ImGui::EndCombo();
		}
	}
	ImGui::End();
}

void App::render(le::Renderer& renderer) const {
	//
	m_drawer.draw(renderer, m_card, {});
}
} // namespace cards
