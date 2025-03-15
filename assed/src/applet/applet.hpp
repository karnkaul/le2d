#pragma once
#include <djson/json.hpp>
#include <imcpp.hpp>
#include <kvf/time.hpp>
#include <le2d/context.hpp>
#include <le2d/file_data_loader.hpp>
#include <le2d/input/listener.hpp>
#include <le2d/renderer.hpp>
#include <le2d/service_locator.hpp>
#include <log.hpp>
#include <save_modal.hpp>
#include <gsl/pointers>

namespace le::assed {
class Applet : public input::Listener {
  public:
	explicit Applet(gsl::not_null<ServiceLocator const*> services);

  protected:
	[[nodiscard]] auto get_services() const -> ServiceLocator const& { return *m_services; }
	[[nodiscard]] auto get_context() const -> Context& { return get_services().get<Context>(); }
	[[nodiscard]] auto get_data_loader() const -> FileDataLoader const& { return get_services().get<FileDataLoader>(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::vec2 { return get_context().framebuffer_size(); }

	[[nodiscard]] auto load_bytes(Uri const& uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto load_string(Uri const& uri) const -> std::string;
	[[nodiscard]] auto load_json(Uri const& uri) const -> dj::Json;

	void wait_idle() const { get_context().get_render_window().get_render_device().get_device().waitIdle(); }

	void raise_error(std::string message, std::string title = "Error!");

	SaveModal m_save_modal{};

  private:
	struct ErrorModal {
		std::string title{};
		std::string message{};

		void operator()() const;
	};

	virtual void setup() {}
	virtual void do_tick(kvf::Seconds dt);
	virtual void tick(kvf::Seconds /*dt*/) {}
	virtual void render(Renderer& /*renderer*/) const {}

	virtual void populate_file_menu() {}
	virtual void populate_menu_bar() {}

	gsl::not_null<ServiceLocator const*> m_services;
	ErrorModal m_error_modal{};

	friend class App;
};
} // namespace le::assed
