#pragma once
#include <glm/vec2.hpp>
#include <le2d/visitor.hpp>
#include <variant>

namespace le {
namespace event {
struct Key {
	int key;
	int action;
	int mods;
};

struct Focus {
	bool in_focus;
};
} // namespace event

using Event = std::variant<event::Key, event::Focus>;
} // namespace le
