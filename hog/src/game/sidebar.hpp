#pragma once
#include <game/level.hpp>
#include <le2d/service_locator.hpp>
#include <ui/scroll_view.hpp>

namespace hog {
class Sidebar {
  public:
	explicit Sidebar(le::ServiceLocator const& services);

	void initialize_for(Level const& level);

	void set_framebuffer_size(glm::vec2 size);

	void set_collected(std::size_t index, bool collected);

	void tick(kvf::Seconds dt);
	void draw(le::Renderer& renderer) const;

	le::Texture const* tile_bg{};
	le::Texture const* checkbox{};

  private:
	struct Tile : ui::Widget {
		[[nodiscard]] auto get_hitbox() const -> kvf::Rect<> final { return background.bounding_rect(); }
		void draw(le::Renderer& renderer) const final;
		void on_click() final;
		[[nodiscard]] auto get_position() const -> glm::vec2 final { return background.instance.transform.position; }
		void set_position(glm::vec2 position) final;

		le::drawable::Quad background{};
		le::drawable::Sprite sprite{};
		le::drawable::Quad checkbox{};
		bool collected{};
	};

	[[nodiscard]] auto to_tile(Collectible const& collectible, Prop const& prop) const -> std::unique_ptr<Tile>;

	ui::ScrollView m_scroller{};
	std::vector<Tile*> m_tiles{};

	glm::vec2 m_framebuffer_size{};
};
} // namespace hog
