#pragma once
#include "le2d/console/terminal.hpp"
#include "le2d/input/router.hpp"
#include <span>

namespace le::console {
/// \brief Dispatch events to terminal if active else to input router.
class Junction {
  public:
	/// \param router Persistent pointer to input router.
	/// \param terminal Persistent pointer to terminal instance.
	explicit Junction(gsl::not_null<input::Router*> router, gsl::not_null<ITerminal*> terminal) : m_router(router), m_terminal(terminal) {}

	/// \param events Event queue.
	void dispatch(std::span<Event const> events) const;

  private:
	gsl::not_null<input::Router*> m_router;
	gsl::not_null<ITerminal*> m_terminal;
};
} // namespace le::console
