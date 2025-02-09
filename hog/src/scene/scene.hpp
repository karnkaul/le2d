#pragma once
#include <imgui.h>
#include <event_handler.hpp>
#include <kvf/time.hpp>
#include <le2d/renderer.hpp>
#include <le2d/service_locator.hpp>
#include <le2d/unprojector.hpp>

namespace hog::scene {
class Scene : public SubEventHandler {
  public:
	explicit Scene(gsl::not_null<le::ServiceLocator const*> services) : m_services(services) {}

	[[nodiscard]] virtual auto clear_color() const -> kvf::Color { return kvf::black_v; }

	void handle_events(std::span<le::Event const> events);

	virtual void tick(kvf::Seconds /*dt*/) {}
	virtual void render(le::Renderer& /*renderer*/) const {}

	virtual void disengage_input() {}

	[[nodiscard]] auto get_unprojector(le::Transform const& render_view) const -> le::Unprojector;

	[[nodiscard]] auto unproject_point(le::Transform const& render_view, le::ndc::vec2 point) const -> glm::vec2 {
		return get_unprojector(render_view).unproject(point);
	}

  protected:
	gsl::not_null<le::ServiceLocator const*> m_services;
};
} // namespace hog::scene
