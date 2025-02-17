#pragma once
#include <glm/vec2.hpp>
#include <klib/visitor.hpp>
#include <kvf/codepoint.hpp>
#include <le2d/vector_space.hpp>
#include <span>
#include <string>
#include <variant>

namespace le {
namespace event {
struct WindowClose {};

enum class WindowFocus : bool { False, True };

enum class CursorFocus : bool { False, True };

struct FramebufferResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct WindowResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct WindowPos : window::ivec2 {
	using window::ivec2::ivec2;
};

struct CursorPos {
	window::vec2 window{};
	ndc::vec2 normalized{};
};

using Codepoint = kvf::Codepoint;

struct Key {
	int key;
	int action;
	int mods;

	auto operator==(Key const&) const -> bool = default;
};

struct MouseButton {
	int button;
	int action;
	int mods;

	auto operator==(MouseButton const&) const -> bool = default;
};

struct Scroll : glm::vec2 {
	using glm::vec2::vec2;
};

struct Drop {
	std::span<std::string const> paths{};
};

using Event =
	std::variant<WindowClose, WindowFocus, CursorFocus, FramebufferResize, WindowResize, WindowPos, CursorPos, Codepoint, Key, MouseButton, Scroll, Drop>;
} // namespace event

using Event = event::Event;
} // namespace le
