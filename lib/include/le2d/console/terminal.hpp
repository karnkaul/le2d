#pragma once
#include "kvf/time.hpp"
#include "le2d/drawable/drawable.hpp"
#include "le2d/event.hpp"
#include "le2d/tweak/store.hpp"
#include <string_view>

namespace le::console {
class ITerminal : public IDrawable, public tweak::IStore {
  public:
	[[nodiscard]] virtual auto is_null() const -> bool = 0;

	[[nodiscard]] virtual auto is_active() const -> bool = 0;
	virtual void toggle_active() = 0;

	virtual void println(std::string_view text) = 0;
	virtual void printerr(std::string_view text) = 0;

	[[nodiscard]] virtual auto get_background() const -> kvf::Color = 0;
	virtual void set_background(kvf::Color color) = 0;

	virtual auto consume_event(Event const& event) -> bool = 0;

	virtual void tick(kvf::Seconds dt) = 0;
};
} // namespace le::console
