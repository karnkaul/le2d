#pragma once
#include <klib/polymorphic.hpp>
#include <le2d/event/event.hpp>

namespace le::event {
class Dispatch;

class IListener : public klib::Polymorphic {
  public:
	IListener(IListener const&) = delete;
	auto operator=(IListener const&) -> IListener& = delete;

	IListener() = default;

	IListener(IListener&& rhs) noexcept;
	auto operator=(IListener&& rhs) noexcept -> IListener&;

	~IListener();

	virtual auto consume_event(Event const& event) -> bool = 0;

  private:
	Dispatch* m_dispatch{};

	friend class Dispatch;
};
} // namespace le::event
