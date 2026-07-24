#pragma once
#include "cards/game/conductor.hpp"
#include "cards/scene/scene.hpp"

namespace cards::scene {
class Lab : public Scene {
	void initialize() final;
	void tick(kvf::Seconds dt) final;
	void render(le::IRenderer& renderer) const final;

	void bind_mapping();
	void rebuild_deck();

	std::optional<game::Board> m_board{};
	std::optional<game::Conductor> m_conductor{};

	std::shared_ptr<le::input::IMapping> m_mapping{};
};
} // namespace cards::scene
