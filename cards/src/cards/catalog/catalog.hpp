#pragma once
#include "cards/game/world_space.hpp"
#include "cards/types.hpp"
#include "le2d/asset/asset_map.hpp"
#include "le2d/resource/texture.hpp"
#include "le2d/uri.hpp"
#include <unordered_map>

namespace cards {
namespace catalog {
struct Entry {
	le::Uri uri{};
	klib::Ptr<le::ITexture const> texture{};
};

class EntryArray {
  public:
	[[nodiscard]] auto entry_if(std::size_t const index) const -> klib::Ptr<Entry const> {
		if (index >= entries.size()) { return nullptr; }
		return &entries[index];
	}

	[[nodiscard]] auto texture_if(std::size_t const index) const -> klib::Ptr<le::ITexture const> {
		auto const entry = entry_if(index);
		if (!entry) { return nullptr; }
		return entry->texture;
	}

	std::vector<Entry> entries{};
};

class Cover : public EntryArray {};
class Joker : public EntryArray {};
class Overlay : public EntryArray {};

class SuitGroup {
  public:
	explicit SuitGroup(Suit const suit) : m_suit(suit) {}

	[[nodiscard]] auto get_suit() const -> cards::Suit { return m_suit; }

	[[nodiscard]] auto texture_if(Value const value) const -> klib::Ptr<le::ITexture const> {
		auto const it = entries.find(value);
		if (it == entries.end()) { return nullptr; }
		return it->second.texture;
	}

	std::unordered_map<Value, Entry> entries{};

  private:
	Suit m_suit;
};
} // namespace catalog

struct Config {
	[[nodiscard]] constexpr auto get_card_height() const -> float { return n_card_height * world_space_v.y; }
	[[nodiscard]] constexpr auto get_select_offset() const -> float { return n_select_offset * get_card_height(); }
	[[nodiscard]] constexpr auto get_hand_canvas_size() const -> glm::vec2 { return hand_n_canvas_length * world_space_v; }

	std::size_t cover_index{0uz};
	std::size_t select_overlay_index{0uz};

	float hand_n_canvas_length{0.5f};
	float n_card_height{0.2f};
	float n_select_offset{0.2f};

	kvf::Seconds deal_rate{0.02s};
	kvf::Seconds submit_ttl{0.2s};
	kvf::Seconds discard_delay{2s};
	kvf::Seconds discard_ttl{0.2s};
};

class Catalog {
  public:
	class Parser;

	explicit Catalog();

	[[nodiscard]] auto get_cover() const -> catalog::Cover const& { return m_cover; }
	[[nodiscard]] auto get_joker() const -> catalog::Joker const& { return m_joker; }
	[[nodiscard]] auto get_overlay() const -> catalog::Overlay const& { return m_overlay; }
	[[nodiscard]] auto get_suit_group(Suit suit) const -> catalog::SuitGroup const&;

	[[nodiscard]] auto get_cover_texture() const -> klib::Ptr<le::ITexture const> { return m_cover.texture_if(m_config.cover_index); }

	[[nodiscard]] auto get_config() const -> Config const& { return m_config; }

	[[nodiscard]] auto get_texture_uris() const -> std::vector<le::Uri>;
	void assign_textures(le::AssetMap const& asset_map);

  private:
	template <typename Self, typename FuncT>
	void iterate_entries(this Self&& self, FuncT func);

	catalog::Cover m_cover{};
	catalog::Joker m_joker{};
	catalog::Overlay m_overlay{};
	std::unordered_map<Suit, catalog::SuitGroup> m_suit_groups{};
	Config m_config{};
};
} // namespace cards
