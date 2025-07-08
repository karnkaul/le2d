#pragma once
#include <applet/applet.hpp>
#include <kvf/time.hpp>
#include <le2d/context.hpp>
#include <le2d/file_data_loader.hpp>
#include <le2d/input/dispatch.hpp>
#include <le2d/service_locator.hpp>
#include <memory>

namespace le::assed {
class App : public klib::Pinned {
  public:
	explicit App(FileDataLoader data_loader);

	void run();

  private:
	using CreateApplet = std::unique_ptr<Applet> (*)(gsl::not_null<le::ServiceLocator const*>);

	struct Factory {
		klib::CString name{};
		CreateApplet create{};
	};

	void create_factories();

	void swap_applet();
	void handle_events();
	void tick();
	void render();

	void main_menu();
	void file_menu();
	void applet_menu();

	void set_applet(Factory const& factory);

	void try_exit();

	FileDataLoader m_data_loader;

	Context m_context;
	input::Dispatch m_input_dispatch{};
	AssetLoader m_asset_loader{};

	ServiceLocator m_service_locator{};
	std::vector<Factory> m_factories{};

	std::unique_ptr<Applet> m_applet{};
	std::string_view m_next_applet{};
	kvf::DeltaTime m_delta_time{};

	Context::Waiter m_waiter{};
};
} // namespace le::assed
