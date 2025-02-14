#include <le2d/input/dispatch.hpp>
#include <le2d/input/listener.hpp>

namespace le::input {
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
