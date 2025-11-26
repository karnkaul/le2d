#include "le2d/input/listener.hpp"
#include "le2d/input/dispatch.hpp"

namespace le::input {
Listener::Listener(gsl::not_null<Dispatch*> dispatch) : m_dispatch(dispatch) { m_dispatch->attach(this); }

Listener::Listener(Listener&& rhs) noexcept : m_dispatch(rhs.m_dispatch) {
	if (m_dispatch == nullptr) { return; }
	m_dispatch->attach(this);
}

auto Listener::operator=(Listener&& rhs) noexcept -> Listener& {
	if (m_dispatch != rhs.m_dispatch) {
		if (m_dispatch != nullptr) { m_dispatch->detach(this); }
		m_dispatch = rhs.m_dispatch;
		if (m_dispatch != nullptr) { m_dispatch->attach(this); }
	}
	return *this;
}

Listener::~Listener() {
	if (m_dispatch == nullptr) { return; }
	m_dispatch->detach(this);
}
} // namespace le::input
