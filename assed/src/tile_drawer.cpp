#include "tile_drawer.hpp"
#include "imcpp.hpp"
#include "le2d/vector_space.hpp"
#include <imgui.h>
#include <ranges>

namespace le::assed {
auto TileDrawer::create_tile_frame(kvf::Rect<> const& rect) const -> drawable::LineRect {
	auto tile_frame = drawable::LineRect{};
	tile_frame.geometry.create(rect.size());
	tile_frame.instance.transform.position = rect.center();
	tile_frame.instance.tint = style.frame_color;
	return tile_frame;
}

void TileDrawer::setup(std::span<Tile const> tiles, glm::vec2 const texture_size) {
	quad.geometry.create(texture_size);
	tile_frames.clear();
	tile_frames.reserve(tiles.size());
	for (auto const& tile : tiles) {
		auto const rect = uv_to_world(tile.uv, texture_size);
		tile_frames.push_back(create_tile_frame(rect));
	}
	update();
}

void TileDrawer::clear() {
	quad.texture = nullptr;
	tile_frames.clear();
	selected_tile.reset();
}

void TileDrawer::update() {
	if (selected_tile && *selected_tile >= tile_frames.size()) { selected_tile.reset(); }

	for (auto const [index, tile_frame] : std::views::enumerate(tile_frames)) {
		if (selected_tile && *selected_tile == std::size_t(index)) {
			tile_frame.instance.tint = style.selected_color;
			continue;
		}
		tile_frame.instance.tint = style.frame_color;
	}
}

void TileDrawer::draw(IRenderer& renderer) const {
	quad.draw(renderer);

	renderer.set_line_width(style.frame_width);
	auto selected = klib::Ptr<drawable::LineRect const>{};
	for (auto const [index, tile_frame] : std::views::enumerate(tile_frames)) {
		if (selected_tile && *selected_tile == std::size_t(index)) {
			selected = &tile_frame;
			continue;
		}
		tile_frame.draw(renderer);
	}
	if (selected) { selected->draw(renderer); }
}

void TileDrawer::inspect_style() {
	if (imcpp::color_edit("frame color", style.frame_color)) {
		for (auto [index, tile_frame] : std::views::enumerate(tile_frames)) {
			if (selected_tile && *selected_tile == std::size_t(index)) { continue; }
			tile_frame.instance.tint = style.frame_color;
		}
	}
	if (imcpp::color_edit("selected color", style.selected_color)) {
		if (selected_tile) { tile_frames.at(*selected_tile).instance.tint = style.selected_color; }
	}
	ImGui::DragFloat("frame width", &style.frame_width, 0.5f, 1.0f, 100.0f);
}
} // namespace le::assed
