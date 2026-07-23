#include "cards/catalog/catalog.hpp"
#include "cards/catalog/catalog_parser.hpp"
#include "cards/catalog/catalog_resources.hpp"
#include "cards/util.hpp"
#include "le2d/asset/asset_manifest.hpp"
#include "le2d/asset/asset_map.hpp"

namespace cards {
namespace {
template <typename AssetTypeT>
[[nodiscard]] auto to_entry(dj::Json const& uri) -> catalog::Entry<AssetTypeT> {
	return catalog::Entry<AssetTypeT>{.uri = uri.as<std::string>()};
}

template <typename AssetTypeT>
void assign_asset(catalog::Entry<AssetTypeT>& out, le::AssetMap const& asset_map, bool const required = true) {
	out.asset = asset_map.get_asset_if<AssetTypeT>(out.uri);
	if (required && !out.asset) { throw std::runtime_error{std::format("Required asset not found: {}", out.uri.get_string())}; }
}
} // namespace

Catalog::Catalog() {
	for (auto const suit : util::all_suits_v) { m_suit_groups.insert_or_assign(suit, catalog::SuitGroup{suit}); }
}

auto Catalog::get_suit_group(Suit const suit) const -> catalog::SuitGroup const& {
	auto const it = m_suit_groups.find(suit);
	KLIB_ASSERT(it != m_suit_groups.end());
	return it->second;
}

template <typename FuncT>
void Catalog::iterate_texture_entries(this auto&& self, FuncT func) {
	for (auto& entry : self.m_cover.entries) { func(entry); }
	for (auto& entry : self.m_joker.entries) { func(entry); }
	for (auto& [_, suit_group] : self.m_suit_groups) {
		for (auto& [_, entry] : suit_group.entries) { func(entry); }
	}
}

void Catalog::set_timing_type(TimingType const type) {
	m_config.timings = to_timings(type);
	m_timing_type = type;
}

void Catalog::Parser::parse(dj::Json const& json) {
	parse_images(json["images"]);
	parse_fonts(json["fonts"]);
	parse_config(json["config"]);
}

void Catalog::Parser::parse_images(dj::Json const& json) {
	for (auto const& entry : json["covers"].as_array()) { m_catalog.m_cover.entries.push_back(to_entry<le::ITexture>(entry)); }
	for (auto const& entry : json["jokers"].as_array()) { m_catalog.m_joker.entries.push_back(to_entry<le::ITexture>(entry)); }

	for (auto const& [suit, name] : suit_name_map.as_span()) { parse_suit(json[name], suit); }
}

void Catalog::Parser::parse_suit(dj::Json const& json, Suit const suit) {
	auto const it = m_catalog.m_suit_groups.find(suit);
	KLIB_ASSERT(it != m_catalog.m_suit_groups.end());

	auto& group_suit = it->second;
	for (auto const& [key, in_entry] : json.as_object()) {
		auto const value = value_name_map.to_enum(key);
		if (!value) { continue; }
		group_suit.entries.insert_or_assign(*value, to_entry<le::ITexture>(in_entry));
	}
}

void Catalog::Parser::parse_fonts(dj::Json const& json) {
	for (auto const& entry : json.as_array()) { m_catalog.m_fonts.entries.push_back(to_entry<le::IFont>(entry)); }
}

void Catalog::Parser::parse_config(dj::Json const& json) {
	auto& config = m_catalog.m_config;
	if (auto const& cover_index = json["cover_index"]) { from_json(cover_index, config.cover_index); }
	if (auto const& main_font_index = json["main_font_index"]) { from_json(main_font_index, config.main_font_index); }
	if (auto const& mono_font_index = json["mono_font_index"]) { from_json(mono_font_index, config.mono_font_index); }

	if (auto const& n_hand_canvas_length = json["n_hand_canvas_length"]) { from_json(n_hand_canvas_length, config.n_hand_canvas_length); }
	if (auto const& n_card_height = json["n_card_height"]) { from_json(n_card_height, config.n_card_height); }
	if (auto const& n_hover_offset = json["n_hover_offset"]) { from_json(n_hover_offset, config.n_hover_offset); }

	if (auto const& timing_str = json["timing"]; timing_str.is_string()) {
		if (auto const timing_type = timing_type_name_map.to_enum(timing_str.as_string_view())) { m_catalog.set_timing_type(*timing_type); }
	}
}

void Catalog::Resources::populate_manifest(le::AssetManifest& out) const {
	m_catalog.iterate_texture_entries([&](catalog::Entry<le::ITexture> const& entry) { out.add_entry<le::ITexture>(entry.uri); });
	for (auto const& entry : m_catalog.m_fonts.entries) { out.add_entry<le::IFont>(entry.uri); }
}

void Catalog::Resources::assign_assets(le::AssetMap const& asset_map) {
	m_catalog.iterate_texture_entries([&](catalog::Entry<le::ITexture>& entry) { assign_asset(entry, asset_map); });
	for (auto& entry : m_catalog.m_fonts.entries) { assign_asset(entry, asset_map); }
}
} // namespace cards
