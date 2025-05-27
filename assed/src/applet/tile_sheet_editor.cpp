#include <applet/tile_sheet_editor.hpp>
#include <klib/assert.hpp>
#include <klib/fixed_string.hpp>
#include <le2d/asset_loader.hpp>
#include <le2d/file_data_loader.hpp>
#include <le2d/json_io.hpp>
#include <le2d/util.hpp>
#include <ranges>

namespace le::assed {
TileSheetEditor::TileSheetEditor(gsl::not_null<ServiceLocator const*> services) : Applet(services), m_texture(services->get<Context>().create_texture()) {
	m_save_modal.title = "Save TileSheet";
	m_drop_types = FileDrop::Type::Json | FileDrop::Type::Image;
	m_json_types = {json_type_name_v<TileSet>, json_type_name_v<TileSheet>};
}

auto TileSheetEditor::consume_cursor_move(glm::vec2 const cursor) -> bool {
	m_cursor_pos = cursor;
	return true;
}

auto TileSheetEditor::consume_mouse_button(event::MouseButton const& button) -> bool {
	if (button.button != GLFW_MOUSE_BUTTON_1 || button.action != GLFW_RELEASE || button.mods != 0) { return false; }
	on_click();
	return true;
}

void TileSheetEditor::tick(kvf::Seconds const /*dt*/) {
	inspect();

	switch (m_save_modal.update()) {
	case SaveModal::Result::Save: on_save(); break;
	default: break;
	}
}

void TileSheetEditor::render(Renderer& renderer) const {
	renderer.view = m_render_view;
	m_drawer.draw(renderer);
	renderer.view = {};
}

void TileSheetEditor::on_drop(FileDrop const& drop) {
	switch (drop.type) {
	case FileDrop::Type::Image: try_load_texture(drop.uri); break;
	case FileDrop::Type::Json: try_load_json(drop); break;
	default: break;
	}
}

void TileSheetEditor::populate_file_menu() {
	if (ImGui::MenuItem("New")) {
		wait_idle();
		m_drawer.clear();
		m_tiles.clear();
		m_uri = {};
		m_unsaved = false;
		set_title();
	}

	if (ImGui::MenuItem("Save...", nullptr, false, !m_tiles.empty())) { m_save_modal.set_open(m_uri.tile_sheet.get_string()); }
}

void TileSheetEditor::inspect() {
	if (ImGui::Begin("Info")) {
		imcpp::color_edit("background", clear_color);

		ImGui::TextUnformatted(klib::FixedString<256>{"TileSheet: {}", m_uri.tile_sheet.get_string()}.c_str());
		ImGui::TextUnformatted(klib::FixedString<256>{"Texture: {}", m_uri.texture.get_string()}.c_str());
		auto const size = m_texture.get_size();
		ImGui::TextUnformatted(klib::FixedString{"dimensions: {}x{}", size.x, size.y}.c_str());

		ImGui::Separator();
		ImGui::DragInt2("cols x rows", &m_split_dims.x, 1.0f, 1, 100);
		if (ImGui::Button("generate")) { generate_tiles(); }

		if (m_drawer.selected_tile && ImGui::TreeNodeEx("selected", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
			inspect_selected();
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("frame style", ImGuiTreeNodeFlags_Framed)) {
			m_drawer.inspect_style();
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void TileSheetEditor::inspect_selected() {
	auto const texture_size = m_texture.get_size();
	auto& selected_tile = m_tiles.at(*m_drawer.selected_tile);
	ImGui::TextUnformatted(klib::FixedString{"ID: {}", std::to_underlying(selected_tile.id)}.c_str());
	if (imcpp::drag_tex_rect(selected_tile.uv, m_texture.get_size())) {
		auto const rect = uv_to_world(selected_tile.uv, texture_size);
		m_drawer.tile_frames.at(*m_drawer.selected_tile) = m_drawer.create_tile_frame(rect);
		m_drawer.update();
		set_unsaved();
	}
}

void TileSheetEditor::try_load_json(FileDrop const& drop) {
	if (drop.json_type == json_type_name_v<TileSet>) {
		try_load_tileset(drop.uri);
	} else if (drop.json_type == json_type_name_v<TileSheet>) {
		try_load_tilesheet(drop.uri);
	}
}

void TileSheetEditor::try_load_tilesheet(Uri uri) {
	auto loader = AssetLoader{&get_context()};
	auto texture_uri = std::string{};
	auto tile_sheet = loader.load_tile_sheet(uri.get_string(), &texture_uri);
	if (!tile_sheet.is_loaded()) {
		raise_error(std::format("Failed to load TileSheet: '{}'", uri.get_string()));
		return;
	}

	set_tiles(tile_sheet.tile_set.get_tiles());
	set_texture(static_cast<Texture&&>(tile_sheet));
	m_drawer.setup(m_tiles, m_texture.get_size());
	m_uri.texture = std::move(texture_uri);
	m_uri.tile_sheet = std::move(uri);
	m_unsaved = false;
	set_title(m_uri.tile_sheet.get_string());

	log.info("loaded TileSheet: '{}'", m_uri.tile_sheet.get_string());
}

void TileSheetEditor::try_load_tileset(Uri const& uri) {
	auto loader = AssetLoader{&get_context()};
	auto const tile_set = loader.load_tile_set(uri.get_string());
	if (!tile_set.is_loaded()) {
		raise_error(std::format("Failed to load TileSet: '{}'", uri.get_string()));
		return;
	}

	set_tiles(tile_set.get_tiles());
	m_drawer.setup(m_tiles, m_texture.get_size());

	log.info("loaded TileSet: '{}'", uri.get_string());
}

void TileSheetEditor::try_load_texture(Uri uri) {
	auto loader = AssetLoader{&get_context()};
	auto texture = loader.load_texture(uri.get_string());
	if (!texture.is_loaded()) {
		raise_error(std::format("Failed to load Texture: '{}'", uri.get_string()));
		return;
	}

	set_texture(std::move(texture));
	m_drawer.setup(m_tiles, m_texture.get_size());
	m_uri.texture = std::move(uri);

	log.info("loaded Texture: '{}'", m_uri.texture.get_string());
}

void TileSheetEditor::set_tiles(std::span<Tile const> tiles) {
	m_tiles = {tiles.begin(), tiles.end()};
	m_drawer.selected_tile.reset();
}

void TileSheetEditor::set_texture(Texture texture) {
	wait_idle();
	m_texture = std::move(texture);
	m_drawer.quad.create(m_texture.get_size());
	m_drawer.quad.texture = &m_texture;
}

void TileSheetEditor::generate_tiles() {
	m_tiles = util::divide_into_tiles(m_split_dims.y, m_split_dims.x);
	m_drawer.setup(m_tiles, m_texture.get_size());
	set_unsaved();
}

void TileSheetEditor::set_unsaved() {
	if (m_unsaved) { return; }
	m_unsaved = true;
	set_title(m_uri.tile_sheet.get_string());
}

void TileSheetEditor::on_click() {
	auto const cursor_pos = m_cursor_pos / m_render_view.scale;
	for (auto const [index, tile_frame] : std::views::enumerate(m_drawer.tile_frames)) {
		if (!tile_frame.bounding_rect().contains(cursor_pos)) { continue; }

		if (m_drawer.selected_tile && *m_drawer.selected_tile == std::size_t(index)) {
			m_drawer.selected_tile.reset();
		} else {
			m_drawer.selected_tile = std::size_t(index);
		}
		m_drawer.update();
		break;
	}
}

void TileSheetEditor::on_save() {
	auto json = dj::Json{};
	set_json_type<TileSheet>(json);
	to_json(json["texture"], m_uri.texture.get_string());
	auto tile_set = TileSet{};
	tile_set.set_tiles(m_tiles);
	to_json(json["tile_set"], tile_set);
	auto const uri = std::string{m_save_modal.uri_input.as_view()};
	if (!get_data_loader().save_string(to_string(json), uri)) {
		raise_error(std::format("Failed to save TileSheet to: '{}'", m_save_modal.uri_input.as_view()));
		return;
	}
	m_uri.tile_sheet = std::string{m_save_modal.uri_input.as_view()};
	log.info("saved TileSheet: '{}'", m_uri.tile_sheet.get_string());
	raise_dialog(std::format("Saved {}", m_uri.tile_sheet.get_string()), "Success");
	set_title(m_uri.tile_sheet.get_string());
}
} // namespace le::assed
