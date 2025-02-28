#pragma once
#include <le2d/console.hpp>
#include <le2d/context.hpp>
#include <le2d/input/dispatch.hpp>
#include <le2d/service_locator.hpp>
#include <resources.hpp>
#include <scene/switcher.hpp>

namespace hog {
struct App : public scene::ISwitcher {
	explicit App(gsl::not_null<le::IDataLoader const*> data_loader);

	void run();

  private:
	auto get_services() const -> le::ServiceLocator const& final { return m_services; }

	void enqueue_switch(SwitchFunc create_scene) final;

	void tick(kvf::Seconds dt);
	void render(le::Renderer& renderer) const;
	void process_events();

	le::Context m_context;

	std::optional<Resources> m_resources{};
	le::input::Dispatch m_input_dispatch{};

	le::ServiceLocator m_services{};

	kvf::DeltaTime m_delta_time{};
	std::unique_ptr<scene::Scene> m_scene{};
	SwitchFunc m_create_scene{};

	std::optional<le::console::Terminal> m_terminal{};
	bool m_was_terminal_active{};

	kvf::DeviceBlock m_blocker;
};
} // namespace hog
