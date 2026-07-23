#pragma once
#include "cards/game/board.hpp"
#include "cards/game/selector/selector.hpp"
#include "cards/game/state/play_state.hpp"
#include "cards/game/tally.hpp"
#include "cards/scene/scene.hpp"

namespace cards::scene {
class Lab : public Scene {
	void initialize() final;
	void tick(kvf::Seconds dt) final;
	void render(le::IRenderer& renderer) const final;

	void bind_mapping();
	void rebuild_deck();

	void submit_and_advance(Card card);

	void try_submit_next();
	void try_complete_round(kvf::Seconds dt);

	std::optional<game::Board> m_board{};
	std::shared_ptr<le::input::IMapping> m_mapping{};
	game::PlayState m_play_state{};
	game::Tally m_tally{};

	game::PerSeat<std::unique_ptr<game::Selector>> m_selectors{};

	kvf::Seconds m_complete_round_interval{1s};
	kvf::Seconds m_complete_round_remain{};
};
} // namespace cards::scene
