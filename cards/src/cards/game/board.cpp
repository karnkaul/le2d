#include "cards/game/board.hpp"
#include "cards/util.hpp"
#include <ranges>

namespace cards::game {
Board::Board(gsl::not_null<Catalog const*> catalog, Pawn::Builder const& pawn_builder) : m_catalog(catalog) {
	create_pawns(pawn_builder);

	m_indicator.instance.transform.position.y = -0.2f * world_space_v.y;
	m_indicator.geometry.create(30.0f);
	m_indicator.instance.transform.orientation.rotate(glm::radians(180.0f));
}

void Board::draw(le::IRenderer& renderer) const {
	draw_indicator(renderer);
	renderer.set_view({});
	for (auto const& pawn : std::views::reverse(m_pawns.values)) { pawn->draw(renderer); }
	trick.draw(renderer);
}

void Board::tick(kvf::Seconds const dt) {
	for (auto& pawn : m_pawns.values) { pawn->tick(dt); }
}

auto Board::are_hands_empty() const -> bool {
	return std::ranges::all_of(m_pawns.values, [](auto const& pawn) { return pawn->is_hand_empty(); });
}

auto Board::select_submit() -> std::optional<Card> {
	if (!current_seat) { return {}; }
	return m_pawns.at(*current_seat)->select_submit();
}

void Board::create_pawns(Pawn::Builder const& builder) {
	for (auto const seat : util::all_seats_v) { m_pawns.at(seat) = builder.build_for(seat, &play_state); }
}

void Board::draw_indicator(le::IRenderer& renderer) const {
	if (!current_seat) { return; }

	auto render_view = le::Transform{};
	render_view.orientation = util::render_view_orientation(*current_seat);
	renderer.set_view(render_view);
	m_indicator.draw(renderer);
}
} // namespace cards::game
