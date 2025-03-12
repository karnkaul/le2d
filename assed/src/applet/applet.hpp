#pragma once
#include <kvf/time.hpp>
#include <le2d/input/listener.hpp>
#include <le2d/renderer.hpp>
#include <le2d/service_locator.hpp>
#include <gsl/pointers>

namespace le::assed {
class Applet : public input::Listener {
  public:
	explicit Applet(gsl::not_null<ServiceLocator const*> services);

  protected:
	[[nodiscard]] auto get_services() const -> ServiceLocator const& { return *m_services; }

	[[nodiscard]] auto get_framebuffer_size() const -> glm::vec2;

  private:
	virtual void setup() {}
	virtual void tick(kvf::Seconds /*dt*/) {}
	virtual void render(Renderer& /*renderer*/) const {}

	virtual void populate_file_menu() {}
	virtual void populate_menu_bar() {}

	gsl::not_null<ServiceLocator const*> m_services;

	friend class App;
};
} // namespace le::assed
