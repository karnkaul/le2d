#pragma once
#include <glm/vec2.hpp>
#include <klib/visitor.hpp>
#include <kvf/ttf.hpp>
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

using Codepoint = kvf::ttf::Codepoint;
} // namespace event

using Event = std::variant<event::Key, event::Focus, event::Codepoint>;
} // namespace le
