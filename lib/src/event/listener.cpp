#include <le2d/event/dispatch.hpp>
#include <le2d/event/listener.hpp>

namespace le::event {
IListener::IListener(IListener&& rhs) noexcept : m_dispatch(rhs.m_dispatch) {
	if (m_dispatch == nullptr) { return; }
	m_dispatch->attach(this);
}

auto IListener::operator=(IListener&& rhs) noexcept -> IListener& {
	if (m_dispatch != rhs.m_dispatch) {
		if (m_dispatch != nullptr) { m_dispatch->detach(this); }
		m_dispatch = rhs.m_dispatch;
		if (m_dispatch != nullptr) { m_dispatch->attach(this); }
	}
	return *this;
}

IListener::~IListener() {
	if (m_dispatch == nullptr) { return; }
	m_dispatch->detach(this);
}
} // namespace le::event
