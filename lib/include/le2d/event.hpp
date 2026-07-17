#pragma once
#include "klib/visitor.hpp"
#include "kvf/codepoint.hpp"
#include "le2d/vector_space.hpp"
#include <glm/vec2.hpp>
#include <span>
#include <string>
#include <variant>

namespace le {
namespace event {
struct WindowClose {};

enum class WindowFocus : std::int8_t { False, True };

enum class CursorFocus : std::int8_t { False, True };

enum class WindowIconify : std::int8_t { False, True };

struct SwapchainResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct WindowResize : glm::ivec2 {
	using glm::ivec2::ivec2;
};

struct WindowPos : window::ivec2 {
	using window::ivec2::ivec2;
};

struct Drop {
	std::span<std::string const> paths{};
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

using Event = std::variant<WindowClose, WindowFocus, CursorFocus, SwapchainResize, WindowResize, WindowPos, WindowIconify, Drop, CursorPos, Codepoint, Key,
						   MouseButton, Scroll>;
} // namespace event

using Event = event::Event;
} // namespace le
