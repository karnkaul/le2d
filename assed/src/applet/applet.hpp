#pragma once
#include <imgui.h>
#include <djson/json.hpp>
#include <kvf/time.hpp>
#include <le2d/context.hpp>
#include <le2d/input/listener.hpp>
#include <le2d/renderer.hpp>
#include <le2d/service_locator.hpp>
#include <log.hpp>
#include <gsl/pointers>

namespace le::assed {
class Applet : public input::Listener {
  public:
	explicit Applet(gsl::not_null<ServiceLocator const*> services);

  protected:
	[[nodiscard]] auto get_services() const -> ServiceLocator const& { return *m_services; }
	[[nodiscard]] auto get_context() const -> Context& { return get_services().get<Context>(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::vec2 { return get_context().framebuffer_size(); }

	[[nodiscard]] static auto bytes_from_file(klib::CString path) -> std::vector<std::byte>;
	[[nodiscard]] static auto string_from_file(klib::CString path) -> std::string;
	[[nodiscard]] static auto json_from_file(klib::CString path) -> dj::Json;

	void wait_idle() const { get_context().get_render_window().get_render_device().get_device().waitIdle(); }

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
