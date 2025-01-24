#pragma once
#include <glm/vec2.hpp>
#include <klib/visitor.hpp>
#include <kvf/ttf.hpp>
#include <span>
#include <string>
#include <variant>

namespace le {
namespace event {
struct WindowClose {};

enum class WindowFocus : bool { False, True };

enum class CursorFocus : bool { False, True };

using Codepoint = kvf::ttf::Codepoint;

struct FramebufferResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct WindowResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct WindowPos : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct CursorPos {
	glm::vec2 window{};
	glm::vec2 normalized{};
};

struct Key {
	int key;
	int action;
	int mods;
};

struct MouseButton {
	int button;
	int action;
	int mods;
};

struct Scroll : glm::vec2 {
	using glm::vec2::vec2;
};

struct Drop {
	std::span<std::string const> paths{};
};

using Event =
	std::variant<WindowClose, WindowFocus, CursorFocus, Codepoint, FramebufferResize, WindowResize, WindowPos, CursorPos, Key, MouseButton, Scroll, Drop>;
} // namespace event

using Event = event::Event;
} // namespace le
