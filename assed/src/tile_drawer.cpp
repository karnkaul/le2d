#include <imgui.h>
#include <le2d/vector_space.hpp>
#include <tile_drawer.hpp>
#include <ranges>

namespace le::assed {
auto TileDrawer::create_tile_frame(kvf::Rect<> const& rect) const -> drawable::LineRect {
	auto tile_frame = drawable::LineRect{};
	tile_frame.create(rect.size());
	tile_frame.transform.position = rect.center();
	tile_frame.tint = style.frame_color;
	return tile_frame;
}

void TileDrawer::setup(std::span<Tile const> tiles, glm::vec2 const texture_size) {
	tile_frames.clear();
	tile_frames.reserve(tiles.size());
	for (auto const& tile : tiles) {
		auto const rect = uv_to_world(tile.uv, texture_size);
		tile_frames.push_back(create_tile_frame(rect));
	}
	update();
}

void TileDrawer::update() {
	if (selected_tile && *selected_tile >= tile_frames.size()) { selected_tile.reset(); }

	for (auto const [index, tile_frame] : std::views::enumerate(tile_frames)) {
		if (selected_tile && *selected_tile == std::size_t(index)) {
			tile_frame.tint = style.selected_color;
			continue;
		}
		tile_frame.tint = style.frame_color;
	}
}

void TileDrawer::draw(Renderer& renderer) const {
	quad.draw(renderer);

	renderer.set_line_width(style.frame_width);
	drawable::LineRect const* selected{};
	for (auto const [index, tile_frame] : std::views::enumerate(tile_frames)) {
		if (selected_tile && *selected_tile == std::size_t(index)) {
			selected = &tile_frame;
			continue;
		}
		tile_frame.draw(renderer);
	}
	if (selected != nullptr) { selected->draw(renderer); }
}

void TileDrawer::inspect_style() {
	auto color = style.frame_color.to_vec4();
	if (ImGui::ColorEdit4("frame color", &color.x)) {
		style.frame_color = color;
		for (auto [index, tile_frame] : std::views::enumerate(tile_frames)) {
			if (selected_tile && *selected_tile == std::size_t(index)) { continue; }
			tile_frame.tint = style.frame_color;
		}
	}
	color = style.selected_color.to_vec4();
	if (ImGui::ColorEdit4("selected color", &color.x)) {
		style.selected_color = color;
		if (selected_tile) { tile_frames.at(*selected_tile).tint = style.selected_color; }
	}
	ImGui::DragFloat("frame width", &style.frame_width, 0.5f, 1.0f, 100.0f);
}
} // namespace le::assed
