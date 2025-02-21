#pragma once
#include <game/level.hpp>
#include <le2d/drawable/text.hpp>
#include <le2d/service_locator.hpp>
#include <ui/scroll_view.hpp>

namespace le {
class Context;
}

namespace hog {
class Sidebar {
  public:
	explicit Sidebar(le::ServiceLocator const& services);

	void initialize_for(Level const& level);

	[[nodiscard]] auto contains(glm::vec2 cursor) const { return m_scroller.background.bounding_rect().contains(cursor); }
	void hide_popup() { m_popup.tile = nullptr; }

	void collect(std::size_t index);
	void set_collected(std::size_t index, bool collected);

	void tick(kvf::Seconds dt);
	void draw(le::Renderer& renderer) const;

	le::Texture const* tile_bg{};
	le::Texture const* checkbox{};
	le::Font* font{};

	float tile_size{150.0f};
	float right_pad{10.0f};
	glm::vec2 popup_size{350.0f, 120.0f};
	kvf::Color popup_text{kvf::black_v};
	float popup_pad{20.0f};
	float tile_pad_y{20.0f};

  private:
	struct Tile : ui::Widget {
		[[nodiscard]] auto get_hitbox() const -> kvf::Rect<> final { return background.bounding_rect(); }
		void draw(le::Renderer& renderer) const final;
		void on_click() final;
		[[nodiscard]] auto get_position() const -> glm::vec2 final { return background.transform.position; }
		void set_position(glm::vec2 position) final;

		void set_popup();

		Sidebar* sidebar{};
		std::string_view description{};

		le::drawable::Quad background{};
		le::drawable::Sprite sprite{};
		le::drawable::Quad checkbox{};
		bool collected{};
	};

	struct Popup {
		void update();
		void draw(le::Renderer& renderer) const;

		Tile const* tile{};
		le::drawable::Quad background{};
		le::drawable::Text description{};
	};

	[[nodiscard]] auto to_tile(Collectible const& collectible, Prop const& prop) const -> std::unique_ptr<Tile>;

	void resize(glm::vec2 size);
	void layout_tiles();

	gsl::not_null<le::Context const*> m_context;

	ui::ScrollView m_scroller{};
	std::vector<Tile*> m_tiles{};
	Popup m_popup{};

	glm::vec2 m_framebuffer_size{};
};
} // namespace hog
