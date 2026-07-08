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
	explicit Junction(gsl::not_null<input::Router*> router, klib::Ptr<ITerminal> terminal = {}) : m_router(router), m_terminal(terminal) {}

	/// \param events Event queue.
	/// \param framebuffer_size Size of target framebuffer for terminal.
	void dispatch(std::span<Event const> events, glm::ivec2 framebuffer_size) const;

  private:
	gsl::not_null<input::Router*> m_router;
	klib::Ptr<ITerminal> m_terminal;
};
} // namespace le::console
