#pragma once
#include <le2d/event/listener.hpp>
#include <gsl/pointers>
#include <span>
#include <vector>

namespace le::event {
class Dispatch {
  public:
	Dispatch(Dispatch const&) = delete;
	auto operator=(Dispatch const&) = delete;

	Dispatch() = default;
	Dispatch(Dispatch&& rhs) noexcept;
	auto operator=(Dispatch&& rhs) noexcept -> Dispatch&;
	~Dispatch();

	void attach(gsl::not_null<IListener*> listener);
	void detach(gsl::not_null<IListener*> listener);

	void dispatch(std::span<Event const> events) const;

  private:
	void update_listeners(Dispatch* target) const;

	std::vector<IListener*> m_listeners{};
};
} // namespace le::event
