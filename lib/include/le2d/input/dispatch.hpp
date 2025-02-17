#pragma once
#include <le2d/input/listener.hpp>
#include <gsl/pointers>
#include <vector>

namespace le::input {
class Dispatch {
  public:
	Dispatch(Dispatch const&) = delete;
	auto operator=(Dispatch const&) = delete;

	Dispatch() = default;
	Dispatch(Dispatch&& rhs) noexcept;
	auto operator=(Dispatch&& rhs) noexcept -> Dispatch&;
	~Dispatch();

	void attach(gsl::not_null<Listener*> listener);
	void detach(gsl::not_null<Listener*> listener);

	void on_cursor_move(glm::vec2 pos);
	void on_codepoint(event::Codepoint codepoint);
	void on_key(event::Key const& key);
	void on_mouse_button(event::MouseButton const& button);
	void on_scroll(event::Scroll const& scroll);
	void on_drop(event::Drop const& drop);

  private:
	void update_listeners(Dispatch* target) const;

	template <typename FPtr, typename T>
	void dispatch(FPtr fptr, T const& event) const;

	std::vector<Listener*> m_listeners{};
};
} // namespace le::input
