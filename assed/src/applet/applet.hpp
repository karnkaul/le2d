#pragma once
#include <djson/json.hpp>
#include <file_drop.hpp>
#include <imcpp.hpp>
#include <kvf/time.hpp>
#include <le2d/asset_loader.hpp>
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

	kvf::Color clear_color{kvf::black_v};

  protected:
	[[nodiscard]] virtual auto get_name() const -> klib::CString = 0;

	virtual void setup() {}
	virtual void tick(kvf::Seconds dt) = 0;
	virtual void render(Renderer& renderer) const = 0;

	virtual void on_drop(FileDrop const& /*drop*/) {}

	virtual void populate_file_menu() {}
	virtual void populate_menu_bar() {}

	auto consume_drop(event::Drop const& drop) -> bool override;
	auto consume_scroll(event::Scroll const& scroll) -> bool override;

	[[nodiscard]] auto get_services() const -> ServiceLocator const& { return *m_services; }
	[[nodiscard]] auto get_context() const -> Context& { return get_services().get<Context>(); }
	[[nodiscard]] auto get_data_loader() const -> FileDataLoader const& { return get_services().get<FileDataLoader>(); }
	[[nodiscard]] auto get_framebuffer_size() const -> glm::vec2 { return get_context().framebuffer_size(); }

	[[nodiscard]] auto load_bytes(Uri const& uri) const -> std::vector<std::byte>;
	[[nodiscard]] auto load_string(Uri const& uri) const -> std::string;
	[[nodiscard]] auto load_json(Uri const& uri) const -> dj::Json;
	[[nodiscard]] auto create_asset_loader() const -> AssetLoader { return AssetLoader{&get_data_loader(), &get_context()}; }

	void wait_idle() const { get_context().get_render_window().get_render_device().get_device().waitIdle(); }

	void raise_dialog(std::string message, std::string title);
	void raise_error(std::string message) { raise_dialog(std::move(message), "Error!"); }

	void set_title(std::string_view asset_uri = {}) const;

	FileDrop::Type m_drop_types{};
	std::vector<std::string_view> m_json_types{};

	SaveModal m_save_modal{};
	bool m_unsaved{};

	Transform m_render_view{};
	float m_zoom_speed{0.1f};

  private:
	struct Dialog {
		std::string title{};
		std::string message{};

		void operator()() const;
	};

	struct ConfirmExitDialog {
		static constexpr klib::CString label_v{"Confirm Exit"};

		enum class Result : std::int8_t { None, Quit, Cancel };

		auto operator()() const -> Result;
	};

	void do_setup();
	void do_tick(kvf::Seconds dt);
	[[nodiscard]] auto try_exit() -> bool;

	gsl::not_null<ServiceLocator const*> m_services;
	Dialog m_dialog{};
	ConfirmExitDialog m_confirm_exit{};
	bool m_open_confirm_exit{};

	friend class App;
};
} // namespace le::assed
