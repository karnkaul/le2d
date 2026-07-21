#pragma once
#include "cards/board.hpp"
#include "cards/scene/scene.hpp"

namespace cards::scene {
class Lab : public Scene {
	void initialize() final;
	void tick(kvf::Seconds dt) final;
	void render(le::IRenderer& renderer) const final;

	void build_deck();

	std::optional<Board> m_board{};
};
} // namespace cards::scene
