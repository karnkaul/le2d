#include "cards/catalog/catalog.hpp"
#include "cards/catalog/catalog_parser.hpp"
#include "cards/catalog/catalog_resources.hpp"
#include "cards/util.hpp"
#include "le2d/asset/asset_manifest.hpp"
#include "le2d/asset/asset_map.hpp"

namespace cards {
namespace {
template <typename AssetTypeT>
void set_uri(dj::Json const& uri, catalog::Entry<AssetTypeT>& out) {
	out.uri = uri.as<std::string>();
	if (out.uri.get_string().empty()) { throw std::runtime_error{"Catalog::Parser : empty URI in catalog"}; }
}

template <typename AssetTypeT>
void assign_asset(catalog::Entry<AssetTypeT>& out, le::AssetMap const& asset_map, bool const required = true) {
	out.asset = asset_map.get_asset_if<AssetTypeT>(out.uri);
	if (required && !out.asset) { throw std::runtime_error{std::format("Required asset not found: {}", out.uri.get_string())}; }
}

void from_json(dj::Json const& json, std::optional<std::size_t>& out) {
	if (!json.is_number()) { return; }
	out = json.as<std::size_t>();
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
	parse_audio(json["audio"]);
	parse_config(json["config"]);
}

void Catalog::Parser::parse_images(dj::Json const& json) {
	for (auto const& entry : json["covers"].as_array()) { set_uri(entry, m_catalog.m_cover.entries.emplace_back()); }
	for (auto const& entry : json["jokers"].as_array()) { set_uri(entry, m_catalog.m_joker.entries.emplace_back()); }

	for (auto const& [suit, name] : suit_name_map.as_span()) { parse_suit(json[name], suit); }
}

void Catalog::Parser::parse_suit(dj::Json const& json, Suit const suit) {
	auto const it = m_catalog.m_suit_groups.find(suit);
	KLIB_ASSERT(it != m_catalog.m_suit_groups.end());

	auto& suit_group = it->second;
	for (auto const value : util::all_values_v) {
		auto const value_name = value_name_map.to_name(value);
		auto const& entry = json[value_name];
		if (!entry.is_string()) {
			throw std::runtime_error{std::format("Catalog::Parser : Missing required texture for {}-{}", suit_name_map.to_name(suit), value_name)};
		}
		set_uri(entry, suit_group.entries[value]);
	}
}

void Catalog::Parser::parse_fonts(dj::Json const& json) {
	for (auto const& entry : json.as_array()) { set_uri(entry, m_catalog.m_fonts.entries.emplace_back()); }
}

void Catalog::Parser::parse_audio(dj::Json const& json) {
	for (auto const& entry : json.as_array()) { set_uri(entry, m_catalog.m_audio.entries.emplace_back()); }
}

void Catalog::Parser::parse_config(dj::Json const& json) {
	auto& config = m_catalog.m_config;
	if (auto const& indices = json["indices"]) {
		if (auto const& cover = indices["cover"]) { from_json(cover, config.indices.cover); }
		if (auto const& main_font = indices["main_font"]) { from_json(main_font, config.indices.main_font); }
		if (auto const& mono_font = indices["mono_font"]) { from_json(mono_font, config.indices.mono_font); }
		if (auto const& discard_sfx = indices["discard_sfx"]) { from_json(discard_sfx, config.indices.discard_sfx); }
		if (auto const& submit_sfx = indices["submit_sfx"]) { from_json(submit_sfx, config.indices.submit_sfx); }
		if (auto const& distribute_sfx = indices["distribute_sfx"]) { from_json(distribute_sfx, config.indices.distribute_sfx); }
	}

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
	for (auto const& entry : m_catalog.m_audio.entries) { out.add_entry<le::IAudioBuffer>(entry.uri); }
}

void Catalog::Resources::assign_assets(le::AssetMap const& asset_map) {
	m_catalog.iterate_texture_entries([&](catalog::Entry<le::ITexture>& entry) { assign_asset(entry, asset_map); });
	for (auto& entry : m_catalog.m_fonts.entries) { assign_asset(entry, asset_map); }
	for (auto& entry : m_catalog.m_audio.entries) { assign_asset(entry, asset_map); }
}
} // namespace cards
