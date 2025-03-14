#pragma once
#include <applet/applet.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/texture.hpp>

namespace le::assed {
class TilesetEditor : public Applet {
  public:
	explicit TilesetEditor(gsl::not_null<ServiceLocator const*> services);

  private:
	auto consume_scroll(event::Scroll const& scroll) -> bool final;
	auto consume_drop(event::Drop const& drop) -> bool final;

	void tick(kvf::Seconds dt) final;
	void render(Renderer& renderer) const final;

	void try_load_image(std::string path);

	TileSheet m_tile_sheet;
	drawable::Quad m_quad{};

	std::string m_loaded_path{"[none]"};
	Transform m_render_view{};

	float m_zoom_speed{0.1f};
};
} // namespace le::assed
