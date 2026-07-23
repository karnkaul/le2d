#pragma once
#include "cards/game/round.hpp"

namespace cards::game {
class Tally {
  public:
	class IPointer;

	[[nodiscard]] auto to_card_point(Value value) const -> CardPoint;

	void triage(Round const& round);

	klib::Ptr<IPointer const> custom_pointer{};
	PerSeat<CardPoint> points{};
};

class Tally::IPointer : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto to_card_point(Value value) const -> CardPoint = 0;
};
} // namespace cards::game
