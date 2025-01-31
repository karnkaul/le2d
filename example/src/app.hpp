#pragma once
#include <kvf/time.hpp>
#include <le2d/asset/store.hpp>
#include <le2d/console.hpp>
#include <le2d/context.hpp>
#include <le2d/drawable/shape.hpp>

namespace le::example {
struct App {
	explicit App(gsl::not_null<IDataLoader const*> data_loader);

	void run();

  private:
	struct HeldKeys {
		bool left{};
		bool right{};
	};

	void load_fonts();
	void create_textures();
	void tick(kvf::Seconds dt);
	void render(Renderer& renderer) const;
	void process_events();

	Context m_context;

	asset::Store m_asset_store{};

	std::vector<Texture> m_textures{};
	drawable::Quad m_quad{};

	kvf::DeltaTime m_delta_time{};
	HeldKeys m_held_keys{};
	Transform m_render_view{};

	std::optional<console::Terminal> m_terminal{};

	drawable::Text m_text{};

	kvf::DeviceBlock m_blocker;
};
} // namespace le::example
