#pragma once
#include <kvf/time.hpp>
#include <le2d/asset/store.hpp>
#include <le2d/console.hpp>
#include <le2d/context.hpp>
#include <le2d/drawable/shape.hpp>

namespace hog {
struct App {
	explicit App(gsl::not_null<le::IDataLoader const*> data_loader);

	void run();

  private:
	struct HeldKeys {
		bool left{};
		bool right{};
	};

	void load_fonts();
	void create_textures();
	void tick(kvf::Seconds dt);
	void render(le::Renderer& renderer) const;
	void process_events();

	le::Context m_context;

	le::asset::Store m_asset_store{};

	std::vector<le::Texture> m_textures{};
	le::drawable::Quad m_quad{};

	kvf::DeltaTime m_delta_time{};
	HeldKeys m_held_keys{};
	le::Transform m_render_view{};

	std::optional<le::console::Terminal> m_terminal{};

	le::drawable::Text m_text{};

	kvf::DeviceBlock m_blocker;
};
} // namespace hog
