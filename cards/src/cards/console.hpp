#pragma once
#include "le2d/console/terminal.hpp"
#include "le2d/tweak/tweakable.hpp"

namespace cards {
struct Console {
	std::unique_ptr<le::console::ITerminal> terminal{};

	le::Tweakable<std::string> timing{};
};
} // namespace cards
