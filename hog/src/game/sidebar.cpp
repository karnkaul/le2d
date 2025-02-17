#include <game/sidebar.hpp>
#include <le2d/asset/store.hpp>
#include <le2d/context.hpp>
#include <le2d/input/dispatch.hpp>

namespace hog {
Sidebar::Sidebar(le::ServiceLocator const& services) : m_context(&services.get<le::Context>()), m_scroller(&services.get<le::input::Dispatch>()) {
	m_scroller.background.instance.tint.w = 0x77;
	resize(m_context->framebuffer_size());
}

void Sidebar::initialize_for(Level const& level) {
	m_tiles.clear();
	m_scroller.clear_widgets();
	for (auto const& collectible : level.collectibles) {
		auto tile = to_tile(collectible, level.props.at(collectible.prop_index));
		m_tiles.push_back(tile.get());
		m_scroller.add_widget(std::move(tile));
	}
}

void Sidebar::set_collected(std::size_t const index, bool const collected) { m_tiles.at(index)->collected = collected; }

void Sidebar::tick(kvf::Seconds const dt) {
	glm::vec2 const framebuffer_size = m_context->framebuffer_size();
	if (kvf::is_positive(framebuffer_size) && m_framebuffer_size != framebuffer_size) { resize(framebuffer_size); }
	m_scroller.tick(dt);
}

void Sidebar::draw(le::Renderer& renderer) const { m_scroller.draw(renderer); }

auto Sidebar::to_tile(Collectible const& collectible, Prop const& prop) const -> std::unique_ptr<Tile> {
	auto tile = std::make_unique<Tile>();
	auto const tile_size = tile_bg == nullptr ? glm::ivec2{100} : tile_bg->get_size();
	tile->background.create(tile_size);
	tile->background.texture = tile_bg;
	tile->sprite.set_base_size(0.9f * tile->background.get_size());
	tile->sprite.set_texture(prop.sprite.get_texture(), prop.sprite.get_uv());
	tile->checkbox.create(0.5f * tile->background.get_size());
	tile->checkbox.texture = checkbox;
	tile->collected = collectible.collected;
	return tile;
}

void Sidebar::resize(glm::vec2 const size) {
	m_framebuffer_size = size;
	m_scroller.background.create({150.0f, m_framebuffer_size.y});
	m_scroller.background.instance.transform.position.x = 0.5f * m_framebuffer_size.x - 80.0f;
	m_scroller.reposition_widgets();
}

void Sidebar::Tile::draw(le::Renderer& renderer) const {
	background.draw(renderer);
	sprite.draw(renderer);
	if (collected) { checkbox.draw(renderer); }
}

void Sidebar::Tile::on_click() {
	// TODO: description
}

void Sidebar::Tile::set_position(glm::vec2 const position) {
	background.instance.transform.position = sprite.instance.transform.position = checkbox.instance.transform.position = position;
}
} // namespace hog
