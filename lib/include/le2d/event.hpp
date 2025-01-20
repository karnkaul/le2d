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

enum class Focus : bool { False, True };

using Codepoint = kvf::ttf::Codepoint;

struct FramebufferResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct WindowResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};
} // namespace event

using Event = std::variant<event::Key, event::Focus, event::Codepoint, event::FramebufferResize, event::WindowResize>;
} // namespace le
