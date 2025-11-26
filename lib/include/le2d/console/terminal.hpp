#pragma once
#include "le2d/drawable/drawable.hpp"
#include "le2d/event.hpp"
#include "le2d/resource/font.hpp"
#include "le2d/tweak/store.hpp"
#include <klib/concepts.hpp>
#include <kvf/time.hpp>
#include <cstdint>
#include <string_view>

namespace le::console {
enum class StateChange : std::int8_t { None, Activated, Deactivated };

class ITerminal : public IDrawable, public tweak::IStore {
  public:
	[[nodiscard]] virtual auto is_null() const -> bool = 0;

	[[nodiscard]] virtual auto is_active() const -> bool = 0;
	virtual void toggle_active() = 0;

	virtual void println(std::string_view text) = 0;
	virtual void printerr(std::string_view text) = 0;

	[[nodiscard]] virtual auto get_background() const -> kvf::Color = 0;
	virtual void set_background(kvf::Color color) = 0;

	virtual auto handle_events(glm::vec2 framebuffer_size, std::span<Event const> events) -> StateChange = 0;

	virtual void tick(kvf::Seconds dt) = 0;
};
} // namespace le::console
