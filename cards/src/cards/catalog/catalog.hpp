#pragma once
#include "cards/game/timings.hpp"
#include "cards/game/world_space.hpp"
#include "cards/types.hpp"
#include "le2d/resource/audio_buffer.hpp"
#include "le2d/resource/font.hpp"
#include "le2d/resource/texture.hpp"
#include "le2d/uri.hpp"
#include <unordered_map>

namespace cards {
namespace catalog {
template <std::derived_from<le::IAsset> AssetTypeT>
struct Entry {
	le::Uri uri{};
	klib::Ptr<AssetTypeT> asset{};
};

template <std::derived_from<le::IAsset> AssetTypeT>
class EntryArray {
  public:
	[[nodiscard]] auto entry_if(std::optional<std::size_t> const index) const -> klib::Ptr<Entry<AssetTypeT> const> {
		if (!index || *index >= entries.size()) { return nullptr; }
		return &entries[*index];
	}

	[[nodiscard]] auto asset_if(std::optional<std::size_t> const index) const -> klib::Ptr<AssetTypeT> {
		auto const entry = entry_if(index);
		if (!entry) { return nullptr; }
		return entry->asset;
	}

	std::vector<Entry<AssetTypeT>> entries{};
};

class Cover : public EntryArray<le::ITexture> {};
class Joker : public EntryArray<le::ITexture> {};

class SuitGroup {
  public:
	explicit SuitGroup(Suit const suit) : m_suit(suit) {}

	[[nodiscard]] auto get_suit() const -> cards::Suit { return m_suit; }

	[[nodiscard]] auto texture_if(Value const value) const -> klib::Ptr<le::ITexture const> {
		auto const it = entries.find(value);
		if (it == entries.end()) { return nullptr; }
		return it->second.asset;
	}

	std::unordered_map<Value, Entry<le::ITexture>> entries{};

  private:
	Suit m_suit;
};

class Fonts : public EntryArray<le::IFont> {};
class Audio : public EntryArray<le::IAudioBuffer> {};
} // namespace catalog

struct Config {
	[[nodiscard]] constexpr auto get_card_height() const -> float { return n_card_height * world_space_v.y; }
	[[nodiscard]] constexpr auto get_select_offset() const -> float { return n_hover_offset * get_card_height(); }
	[[nodiscard]] constexpr auto get_hand_canvas_size() const -> glm::vec2 { return n_hand_canvas_length * world_space_v; }

	struct {
		std::size_t cover{0uz};

		// TODO: optional<size_t>
		std::optional<std::size_t> main_font{};
		std::optional<std::size_t> mono_font{};
		std::optional<std::size_t> discard_sfx{};
		std::optional<std::size_t> submit_sfx{};
		std::optional<std::size_t> distribute_sfx{};
	} indices{};

	// proportion of world width/height used by cards in a hand.
	float n_hand_canvas_length{0.5f};
	// proportion of world height.
	float n_card_height{0.2f};
	// proporition of card height to offset hovered card.
	float n_hover_offset{0.2f};

	Timings timings{timing::normal_v};
};

class Catalog {
  public:
	class Parser;
	class Resources;

	explicit Catalog();

	[[nodiscard]] auto get_cover() const -> catalog::Cover const& { return m_cover; }
	[[nodiscard]] auto get_joker() const -> catalog::Joker const& { return m_joker; }
	[[nodiscard]] auto get_suit_group(Suit suit) const -> catalog::SuitGroup const&;

	[[nodiscard]] auto get_cover_texture() const -> klib::Ptr<le::ITexture const> { return m_cover.asset_if(m_config.indices.cover); }

	[[nodiscard]] auto get_fonts() const -> catalog::Fonts const& { return m_fonts; }
	[[nodiscard]] auto get_main_font() const -> klib::Ptr<le::IFont> { return m_fonts.asset_if(m_config.indices.main_font); }
	[[nodiscard]] auto get_mono_font() const -> klib::Ptr<le::IFont> { return m_fonts.asset_if(m_config.indices.mono_font); }

	[[nodiscard]] auto get_audio() const -> catalog::Audio const& { return m_audio; }
	[[nodiscard]] auto get_discard_sfx() const -> klib::Ptr<le::IAudioBuffer const> { return m_audio.asset_if(m_config.indices.discard_sfx); }
	[[nodiscard]] auto get_submit_sfx() const -> klib::Ptr<le::IAudioBuffer const> { return m_audio.asset_if(m_config.indices.submit_sfx); }
	[[nodiscard]] auto get_distribute_sfx() const -> klib::Ptr<le::IAudioBuffer const> { return m_audio.asset_if(m_config.indices.distribute_sfx); }

	[[nodiscard]] auto get_config() const -> Config const& { return m_config; }

	[[nodiscard]] auto get_timing_type() const -> TimingType { return m_timing_type; }
	void set_timing_type(TimingType type);

  private:
	template <typename FuncT>
	void iterate_texture_entries(this auto&& self, FuncT func);

	catalog::Cover m_cover{};
	catalog::Joker m_joker{};
	std::unordered_map<Suit, catalog::SuitGroup> m_suit_groups{};
	catalog::Fonts m_fonts{};
	catalog::Audio m_audio{};
	Config m_config{};
	TimingType m_timing_type{};
};
} // namespace cards
