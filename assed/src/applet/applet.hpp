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

	virtual void setup() {}
	virtual void tick(kvf::Seconds /*dt*/) {}
	virtual void render(Renderer& /*renderer*/) const {}

	[[nodiscard]] virtual auto has_unsaved_changes() const -> bool { return false; }
	virtual void on_save() {}

	virtual void populate_menu_bar() {}

  protected:
	[[nodiscard]] auto get_services() const -> ServiceLocator const& { return *m_services; }

	[[nodiscard]] auto get_framebuffer_size() const -> glm::vec2;

  private:
	gsl::not_null<ServiceLocator const*> m_services;
};
} // namespace le::assed
