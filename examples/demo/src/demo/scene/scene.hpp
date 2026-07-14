#pragma once
#include "klib/base_types.hpp"
#include "kvf/time.hpp"
#include "le2d/context.hpp"
#include "le2d/file_data_loader.hpp"

namespace demo {
class Scene : public klib::Polymorphic {
  public:
	explicit Scene(gsl::not_null<le::Context*> context, gsl::not_null<le::FileDataLoader const*> data_loader, std::string_view name)
		: m_context(context), m_data_loader(data_loader), m_name(name) {}

	[[nodiscard]] auto get_name() const -> std::string_view { return m_name; }

	void tick_frame(kvf::Seconds dt) { tick(dt); }
	void render_frame() const;

  protected:
	[[nodiscard]] auto get_context() const -> le::Context& { return *m_context; }
	[[nodiscard]] auto get_data_loader() const -> le::IDataLoader const& { return *m_data_loader; }
	[[nodiscard]] auto get_file_data_loader() const -> le::FileDataLoader const& { return *m_data_loader; }

	virtual void tick(kvf::Seconds dt);

	virtual void render_custom_passes() const {}
	virtual void render_main_pass([[maybe_unused]] le::IRenderer& renderer) const {}

	kvf::Color m_main_pass_clear{kvf::black_v};

  private:
	gsl::not_null<le::Context*> m_context;
	gsl::not_null<le::FileDataLoader const*> m_data_loader;
	std::string_view m_name{};
};
} // namespace demo
