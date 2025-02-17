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
	m_popup.background.create(popup_size);
	m_popup.description.instance.tint = popup_text;

	m_tiles.clear();
	m_scroller.clear_widgets();
	for (auto const& collectible : level.collectibles) {
		auto tile = to_tile(collectible, level.props.at(collectible.prop_index));
		tile->sidebar = this;
		m_tiles.push_back(tile.get());
		m_scroller.add_widget(std::move(tile));
	}
}

void Sidebar::collect(std::size_t const index) {
	auto* tile = m_tiles.at(index);
	tile->collected = true;
	tile->set_popup();
}

void Sidebar::set_collected(std::size_t const index, bool const collected) { m_tiles.at(index)->collected = collected; }

void Sidebar::tick(kvf::Seconds const dt) {
	glm::vec2 const framebuffer_size = m_context->framebuffer_size();
	if (kvf::is_positive(framebuffer_size) && m_framebuffer_size != framebuffer_size) { resize(framebuffer_size); }
	m_scroller.tick(dt);
	m_popup.update();
}

void Sidebar::draw(le::Renderer& renderer) const {
	m_scroller.draw(renderer);
	m_popup.draw(renderer);
}

auto Sidebar::to_tile(Collectible const& collectible, Prop const& prop) const -> std::unique_ptr<Tile> {
	auto tile = std::make_unique<Tile>();
	tile->description = collectible.description;
	tile->background.create(glm::vec2{tile_size});
	tile->background.texture = tile_bg;
	tile->sprite.set_base_size(0.9f * glm::vec2{tile_size});
	tile->sprite.set_texture(prop.sprite.get_texture(), prop.sprite.get_uv());
	tile->checkbox.create(0.5f * glm::vec2{tile_size});
	tile->checkbox.texture = checkbox;
	tile->collected = collectible.collected;
	return tile;
}

void Sidebar::resize(glm::vec2 const size) {
	m_framebuffer_size = size;
	m_scroller.background.create({tile_size, m_framebuffer_size.y});
	m_scroller.background.instance.transform.position.x = 0.5f * (m_framebuffer_size.x - tile_size) - right_pad;
	m_scroller.reposition_widgets();
}

void Sidebar::Tile::draw(le::Renderer& renderer) const {
	background.draw(renderer);
	sprite.draw(renderer);
	if (collected) { checkbox.draw(renderer); }
}

void Sidebar::Tile::on_click() {
	if (sidebar->m_popup.tile == this) {
		sidebar->hide_popup();
		return;
	}

	if (collected) { return; }
	set_popup();
}

void Sidebar::Tile::set_position(glm::vec2 const position) {
	background.instance.transform.position = sprite.instance.transform.position = checkbox.instance.transform.position = position;
}

void Sidebar::Tile::set_popup() {
	if (sidebar->font != nullptr) {
		auto const params = le::TextParams{
			.height = le::TextHeight{20},
			.expand = le::TextExpand::eBoth,
		};
		sidebar->m_popup.description.set_string(*sidebar->font, description, params);
	}
	sidebar->m_popup.tile = this;
	sidebar->m_popup.update();
}

void Sidebar::Popup::update() {
	if (tile == nullptr) { return; }
	auto const dx = (0.5f * (tile->sidebar->m_scroller.background.get_size().x + background.get_size().x)) + 20.0f;
	background.instance.transform.position = tile->get_position();
	background.instance.transform.position.x -= dx;

	description.instance.transform.position = background.instance.transform.position;
}

void Sidebar::Popup::draw(le::Renderer& renderer) const {
	if (tile == nullptr) { return; }
	background.draw(renderer);
	description.draw(renderer);
}
} // namespace hog
