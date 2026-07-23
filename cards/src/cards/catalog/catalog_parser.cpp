#include "cards/catalog/catalog_parser.hpp"

namespace cards {
namespace {
[[nodiscard]] auto to_entry(dj::Json const& uri) -> catalog::Entry { return catalog::Entry{.uri = uri.as<std::string>()}; }

void from_json(dj::Json const& json, kvf::Seconds& out) { out = kvf::Seconds{json.as<float>()}; }
} // namespace

void Catalog::Parser::parse(dj::Json const& json) {
	parse_images(json["images"]);
	parse_config(json["config"]);
}

void Catalog::Parser::parse_images(dj::Json const& json) {
	for (auto const& entry : json["covers"].as_array()) { m_catalog->m_cover.entries.push_back(to_entry(entry)); }
	for (auto const& entry : json["jokers"].as_array()) { m_catalog->m_joker.entries.push_back(to_entry(entry)); }
	for (auto const& entry : json["overlays"].as_array()) { m_catalog->m_overlay.entries.push_back(to_entry(entry)); }

	for (auto const& [suit, name] : suit_name_map.as_span()) { parse_suit(json[name], suit); }
}

void Catalog::Parser::parse_suit(dj::Json const& json, Suit const suit) {
	auto const it = m_catalog->m_suit_groups.find(suit);
	KLIB_ASSERT(it != m_catalog->m_suit_groups.end());

	auto& group_suit = it->second;
	for (auto const& [key, in_entry] : json.as_object()) {
		auto const value = value_name_map.to_enum(key);
		if (!value) { continue; }
		group_suit.entries.insert_or_assign(*value, to_entry(in_entry));
	}
}

void Catalog::Parser::parse_config(dj::Json const& json) {
	auto& config = m_catalog->m_config;
	if (auto const& cover_index = json["cover_index"]) { from_json(cover_index, config.cover_index); }
	if (auto const& select_overlay_index = json["select_overlay_index"]) { from_json(select_overlay_index, config.select_overlay_index); }
	if (auto const& hand_n_canvas_length = json["hand_n_canvas_length"]) { from_json(hand_n_canvas_length, config.hand_n_canvas_length); }
	if (auto const& n_card_height = json["n_card_height"]) { from_json(n_card_height, config.n_card_height); }
	if (auto const& n_select_offset = json["n_select_offset"]) { from_json(n_select_offset, config.n_select_offset); }
	if (auto const& deal_rate = json["deal_rate_secs"]) { from_json(deal_rate, config.deal_rate); }
	if (auto const& submit_ttl = json["submit_ttl_secs"]) { from_json(submit_ttl, config.submit_ttl); }
	if (auto const& discard_delay = json["discard_delay_secs"]) { from_json(discard_delay, config.discard_delay); }
	if (auto const& discard_ttl = json["discard_ttl_secs"]) { from_json(discard_ttl, config.discard_ttl); }
}
} // namespace cards
