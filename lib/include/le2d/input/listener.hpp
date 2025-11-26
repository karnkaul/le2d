#pragma once
#include "le2d/event.hpp"
#include <klib/base_types.hpp>
#include <gsl/pointers>

namespace le::input {
class Dispatch;

class Listener : public klib::Polymorphic {
  public:
	Listener(Listener const&) = delete;
	auto operator=(Listener const&) -> Listener& = delete;

	Listener() = default;

	explicit Listener(gsl::not_null<Dispatch*> dispatch);

	Listener(Listener&& rhs) noexcept;
	auto operator=(Listener&& rhs) noexcept -> Listener&;

	~Listener() override;

	virtual auto consume_cursor_move(glm::vec2 /*pos*/) -> bool { return false; }
	virtual auto consume_codepoint(event::Codepoint /*codepoint*/) -> bool { return false; }
	virtual auto consume_key(event::Key const& /*key*/) -> bool { return false; }
	virtual auto consume_mouse_button(event::MouseButton const& /*button*/) -> bool { return false; }
	virtual auto consume_scroll(event::Scroll const& /*scroll*/) -> bool { return false; }
	virtual auto consume_drop(event::Drop const& /*drop*/) -> bool { return false; }

	virtual void disengage_input() {}

	[[nodiscard]] auto get_dispatch() const -> Dispatch* { return m_dispatch; }
	[[nodiscard]] auto is_attached() const -> bool { return get_dispatch() != nullptr; }

  private:
	Dispatch* m_dispatch{};

	friend class Dispatch;
};
} // namespace le::input
