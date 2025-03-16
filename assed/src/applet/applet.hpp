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
	[[nodiscard]] virtual auto get_name() const -> klib::CString = 0;

	virtual void setup() {}
	virtual void tick(kvf::Seconds dt) = 0;
	virtual void render(Renderer& renderer) const = 0;

	virtual void populate_file_menu() {}
	virtual void populate_menu_bar() {}

	[[nodiscard]] auto get_services() const -> ServiceLocator const& { return *m_services; }
	[[nodiscard]] auto get_context() const -> Context& { return get_services().get<Context>(); }
	[[nodiscard]] auto get_data_loader() const -> FileDataLoader const& { return get_services().get<FileDataLoader>(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::vec2 { return get_context().framebuffer_size(); }

	[[nodiscard]] auto load_bytes(Uri const& uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto load_string(Uri const& uri) const -> std::string;
	[[nodiscard]] auto load_json(Uri const& uri) const -> dj::Json;

	void wait_idle() const { get_context().get_render_window().get_render_device().get_device().waitIdle(); }

	void raise_dialog(std::string message, std::string title);
	void raise_error(std::string message) { raise_dialog(std::move(message), "Error!"); }

	void set_title() const { set_title({}, false); }
	void set_title(std::string_view asset_uri) const { set_title(asset_uri, m_unsaved); }

	SaveModal m_save_modal{};
	bool m_unsaved{};

  private:
	struct Dialog {
		std::string title{};
		std::string message{};

		void operator()() const;
	};

	void do_setup();
	void do_tick(kvf::Seconds dt);

	void set_title(std::string_view asset_uri, bool unsaved) const;

	gsl::not_null<ServiceLocator const*> m_services;
	Dialog m_dialog{};

	friend class App;
};
} // namespace le::assed
