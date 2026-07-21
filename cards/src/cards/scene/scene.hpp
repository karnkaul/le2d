#pragma once
#include "cards/scene/coordinator.hpp"

namespace cards {
class Scene : public klib::Polymorphic {
  public:
	void initialize_scene(gsl::not_null<scene::ICoordinator*> coordinator);

	void tick_frame(kvf::Seconds dt);

	void render_frame();

  protected:
	virtual void initialize() {}
	virtual void tick([[maybe_unused]] kvf::Seconds dt) {}
	virtual void render([[maybe_unused]] le::IRenderer& renderer) const {}

	[[nodiscard]] auto get_coordinator() const -> scene::ICoordinator& { return *m_coordinator; }

	kvf::Color clear_color{kvf::black_v};

  private:
	klib::Ptr<scene::ICoordinator> m_coordinator{};
};
} // namespace cards
