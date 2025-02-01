#pragma once
#include <le2d/drawable/shape.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
class Lab : public Scene {
  public:
	explicit Lab(gsl::not_null<le::ServiceLocator*> services);

  private:
	struct HeldKeys {
		bool left{};
		bool right{};
	};

	void on_event(le::event::Key key) final;

	void tick(kvf::Seconds dt) final;
	void render(le::Renderer& renderer) const final;
	void reset_events() final;

	void load_fonts();
	void create_textures();

	std::vector<le::Texture> m_textures{};
	le::drawable::Quad m_quad{};

	HeldKeys m_held_keys{};
	le::Transform m_render_view{};
	le::drawable::Text m_text{};
};
} // namespace hog::scene
