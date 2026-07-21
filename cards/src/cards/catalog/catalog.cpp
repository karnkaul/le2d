#include "cards/catalog/catalog.hpp"
#include "cards/util.hpp"

namespace cards {
Catalog::Catalog() {
	for (auto const suit : util::all_suits_v) { m_suit_groups.insert_or_assign(suit, catalog::SuitGroup{suit}); }
}

auto Catalog::get_suit_group(Suit const suit) const -> catalog::SuitGroup const& {
	auto const it = m_suit_groups.find(suit);
	KLIB_ASSERT(it != m_suit_groups.end());
	return it->second;
}

auto Catalog::get_texture_uris() const -> std::vector<le::Uri> {
	auto ret = std::vector<le::Uri>{};
	iterate_entries([&](catalog::Entry const& entry) { ret.push_back(entry.uri); });
	return ret;
}

void Catalog::assign_textures(le::AssetMap const& asset_map) {
	iterate_entries([&](catalog::Entry& entry) { entry.texture = asset_map.get_asset_if<le::ITexture>(entry.uri); });
}

template <typename Self, typename FuncT>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
void Catalog::iterate_entries(this Self&& self, FuncT func) {
	for (auto& entry : self.m_cover.entries) { func(entry); }
	for (auto& entry : self.m_joker.entries) { func(entry); }
	for (auto& entry : self.m_overlay.entries) { func(entry); }
	for (auto& [_, suit_group] : self.m_suit_groups) {
		for (auto& [_, entry] : suit_group.entries) { func(entry); }
	}
}
} // namespace cards
