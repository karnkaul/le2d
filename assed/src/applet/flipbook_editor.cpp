#include <applet/flipbook_editor.hpp>
#include <klib/fixed_string.hpp>
#include <le2d/json_io.hpp>
#include <ranges>

namespace le::assed {
FlipbookEditor::FlipbookEditor(gsl::not_null<ServiceLocator const*> services)
	: Applet(services), m_tile_sheet(services->get<Context>().get_resource_factory().create_tilesheet()) {
	m_drawer.quad.create();
	m_drawer.quad.texture = m_tile_sheet.get();
	m_sprite.set_base_size(glm::vec2{200.0f});
	m_save_modal.title = "Save Flipbook";
	m_drop_types = FileDrop::Type::Json;
	m_json_types = {json_type_name_v<TileSheet>, json_type_name_v<anim::FlipbookAnimation>};
}

void FlipbookEditor::tick(kvf::Seconds const dt) {
	auto const tiles = m_tile_sheet->tile_set.get_tiles();
	if (m_animator.has_animation() && !tiles.empty() && !m_drawer.tile_frames.empty()) {
		auto const anim_dt = m_paused ? 0s : dt;
		m_animator.tick(anim_dt);
		auto const tile_id = m_animator.get_payload();
		auto const it = std::ranges::lower_bound(tiles, tile_id, {}, [](Tile const& t) { return t.id; });
		KLIB_ASSERT(it != tiles.end());
		auto const index = std::size_t(std::distance(tiles.begin(), it));
		KLIB_ASSERT(index < m_drawer.tile_frames.size());
		if (!m_drawer.selected_tile || *m_drawer.selected_tile != index) {
			m_drawer.selected_tile = index;
			m_drawer.update();
		}
		m_sprite.set_tile(m_tile_sheet.get(), tile_id);
	}

	inspect();

	switch (m_save_modal.update()) {
	case SaveModal::Result::Save: on_save(); break;
	default: break;
	}
}

void FlipbookEditor::render(Renderer& renderer) const {
	renderer.view = m_render_view;
	switch (m_display) {
	case Display::TileSheet: m_drawer.draw(renderer); break;
	case Display::Sprite: m_sprite.draw(renderer); break;
	default: break;
	}
	renderer.view = {};
}

void FlipbookEditor::on_drop(FileDrop const& drop) {
	KLIB_ASSERT(drop.type == FileDrop::Type::Json);
	try_load_json(drop);
}

void FlipbookEditor::populate_file_menu() {
	if (ImGui::MenuItem("New")) {
		wait_idle();
		m_drawer.clear();
		m_sprite.set_texture(nullptr);
		m_animator.set_animation(nullptr);
		m_uri = {};
		m_unsaved = false;
		set_title();
	}

	if (ImGui::MenuItem("Save...", nullptr, false, !m_animation.get_timeline().keyframes.empty())) { m_save_modal.set_open(m_uri.tile_sheet.get_string()); }
}

void FlipbookEditor::inspect() {
	if (ImGui::Begin("Info")) {
		imcpp::color_edit("background", clear_color);

		ImGui::TextUnformatted(klib::FixedString<256>{"TileSheet: {}", m_uri.tile_sheet.get_string()}.c_str());
		ImGui::TextUnformatted(klib::FixedString<256>{"Flipbook: {}", m_uri.animation.get_string()}.c_str());
		ImGui::ProgressBar(m_animator.get_progress(), {-FLT_MIN, 0.0f}, "");

		ImGui::Separator();
		inspect_display();

		if (ImGui::TreeNodeEx("generate", ImGuiTreeNodeFlags_Framed)) {
			inspect_generate();
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Timeline", ImGuiTreeNodeFlags_Framed)) {
			inspect_timeline();
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("frame style", ImGuiTreeNodeFlags_Framed)) {
			m_drawer.inspect_style();
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void FlipbookEditor::inspect_display() {
	auto const display_str = display_str_v[m_display];
	if (ImGui::BeginCombo("Display", display_str.data())) {
		for (auto const [index, str] : std::views::enumerate(display_str_v)) {
			if (ImGui::Selectable(str.data(), str == display_str)) { m_display = Display(index); }
		}
		ImGui::EndCombo();
	}
}

void FlipbookEditor::inspect_generate() {
	ImGui::TextUnformatted("Select TileIds");
	m_generate.select_tiles.update("TileIds");
	ImGui::DragFloat("duration", &m_generate.duration, 0.25f, 0.0f, 9999.0f, "%.2fs");
	if (!m_generate.select_tiles.entries.empty() && m_generate.duration > 0.0f && ImGui::Button("generate")) { generate_timeline(); }
}

void FlipbookEditor::inspect_timeline() {
	klib::CString const play_pause = m_paused ? "play" : "pause";
	if (ImGui::Button(play_pause.c_str())) { m_paused = !m_paused; }
	auto elapsed = m_animator.elapsed.count();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	if (ImGui::DragFloat("elapsed", &elapsed, 0.1f, 0.0f, m_animator.get_duration().count())) { m_animator.elapsed = kvf::Seconds{elapsed}; }
	ImGui::TextUnformatted(klib::FixedString{"duration: {:.1f}s", m_animation.get_timeline().duration.count()}.c_str());

	if (ImGui::TreeNode("keyframes")) {
		inspect_keyframes();
		ImGui::TreePop();
	}
}

void FlipbookEditor::inspect_keyframes() {
	for (auto [index, keyframe] : std::views::enumerate(m_animation.get_timeline().keyframes)) {
		if (ImGui::TreeNode(klib::FixedString{"{}", index}.c_str())) {
			ImGui::TextUnformatted(klib::FixedString{"TileID: {}", std::to_underlying(keyframe.payload)}.c_str());
			ImGui::TextUnformatted(klib::FixedString{"timestamp: {:.2f}s", keyframe.timestamp.count()}.c_str());
			ImGui::TreePop();
		}
	}
}

void FlipbookEditor::try_load_json(FileDrop const& drop) {
	if (drop.json_type == json_type_name_v<TileSheet>) {
		try_load_tilesheet(drop.uri);
	} else if (drop.json_type == json_type_name_v<anim::FlipbookAnimation>) {
		try_load_animation(drop.uri);
	}
}

void FlipbookEditor::try_load_tilesheet(Uri uri) {
	auto loader = create_asset_loader();
	auto tile_sheet = loader.load_tile_sheet(uri.get_string());
	if (!tile_sheet) {
		raise_error(std::format("Failed to load TileSheet: '{}'", uri.get_string()));
		return;
	}

	wait_idle();
	m_tile_sheet = std::move(tile_sheet);
	auto const tiles = m_tile_sheet->tile_set.get_tiles();
	m_generate.select_tiles.entries.clear();
	m_generate.select_tiles.entries.reserve(tiles.size());
	for (auto const& tile : tiles) {
		auto tile_entry = imcpp::MultiSelect::Entry{
			.label = std::format("{}", std::to_underlying(tile.id)),
			.is_selected = true,
		};
		m_generate.select_tiles.entries.push_back(std::move(tile_entry));
	}
	m_generate.select_tiles.sync_to_selection();

	m_drawer.setup(m_tile_sheet->tile_set.get_tiles(), m_tile_sheet->get_size());
	m_drawer.quad.texture = m_tile_sheet.get();
	m_sprite.set_tile(m_tile_sheet.get(), TileId{1});

	m_uri.tile_sheet = std::move(uri);
	log.info("loaded TileSheet: '{}'", m_uri.tile_sheet.get_string());
}

void FlipbookEditor::try_load_animation(Uri uri) {
	auto loader = create_asset_loader();
	auto animation = loader.load_flipbook_animation(uri.get_string());
	if (!animation) {
		raise_error(std::format("Failed to load FlipbookAnimation: '{}'", uri.get_string()));
		return;
	}

	m_animation = std::move(*animation);
	m_animator.set_animation(&m_animation);
	m_generate.duration = m_animation.get_timeline().duration.count();
	m_unsaved = false;
	m_paused = false;

	m_uri.animation = std::move(uri);
	set_title(m_uri.animation.get_string());
	log.info("loaded FlipbookAnimation: '{}'", m_uri.animation.get_string());
}

void FlipbookEditor::on_save() {
	auto json = dj::Json{};
	to_json(json, m_animation);
	auto const uri = std::string{m_save_modal.uri_input.as_view()};
	if (!get_data_loader().save_string(to_string(json), uri)) {
		raise_error(std::format("Failed to save Flipbook to: '{}'", m_save_modal.uri_input.as_view()));
		return;
	}
	m_uri.animation = std::string{m_save_modal.uri_input.as_view()};
	log.info("saved Flipbook: '{}'", m_uri.animation.get_string());
	raise_dialog(std::format("Saved {}", m_uri.animation.get_string()), "Success");
	set_title(m_uri.animation.get_string());
}

void FlipbookEditor::generate_timeline() {
	auto timeline = anim::Timeline<TileId>{};
	timeline.duration = kvf::Seconds{m_generate.duration};
	KLIB_ASSERT(!m_generate.select_tiles.entries.empty());
	auto const keyframe_count = m_generate.select_tiles.selection.Size;
	timeline.keyframes.reserve(std::size_t(keyframe_count));
	auto const keyframe_dt = timeline.duration / float(keyframe_count);
	auto timestamp = kvf::Seconds{0s};
	auto const tiles = m_tile_sheet->tile_set.get_tiles();
	for (auto const [index, tile_entry] : std::views::enumerate(m_generate.select_tiles.entries)) {
		if (!tile_entry.is_selected) { continue; }
		auto const tile_id = tiles[std::size_t(index)].id;
		auto const keyframe = anim::Keyframe<TileId>{
			.timestamp = timestamp,
			.payload = tile_id,
		};
		timeline.keyframes.push_back(keyframe);
		timestamp += keyframe_dt;
	}
	m_animation.set_timeline(std::move(timeline));
	m_animator.set_animation(&m_animation);

	m_unsaved = true;
	m_paused = false;
	if (m_uri.animation.get_string().empty()) { m_uri.animation = "untitled_flipbook.json"; }
	set_title(m_uri.animation.get_string());
}
} // namespace le::assed
