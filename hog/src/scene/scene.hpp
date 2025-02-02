#pragma once
#include <event_handler.hpp>
#include <kvf/time.hpp>
#include <le2d/renderer.hpp>
#include <le2d/service_locator.hpp>

namespace hog::scene {
class Scene : public SubEventHandler {
  public:
	explicit Scene(gsl::not_null<le::ServiceLocator const*> services) : m_services(services) {}

	[[nodiscard]] virtual auto clear_color() const -> kvf::Color { return kvf::black_v; }

	virtual void tick(kvf::Seconds /*dt*/) {}
	virtual void render(le::Renderer& /*renderer*/) const {}

	virtual void reset_events() {}

	[[nodiscard]] auto unproject(le::Transform const& render_view, le::ndc::vec2 point) const -> glm::vec2;

  protected:
	gsl::not_null<le::ServiceLocator const*> m_services;
};
} // namespace hog::scene
