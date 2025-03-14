#include <applet/tileset_editor.hpp>
#include <klib/assert.hpp>
#include <klib/fixed_string.hpp>
#include <le2d/file_data_loader.hpp>
#include <algorithm>

namespace le::assed {
namespace {
constexpr auto min_scale_v{0.1f};
constexpr auto max_scale_v{10.0f};
} // namespace

TilesetEditor::TilesetEditor(gsl::not_null<ServiceLocator const*> services) : Applet(services), m_tile_sheet(services->get<Context>().create_tilesheet()) {
	m_quad.create();
	m_quad.texture = &m_tile_sheet;
}

auto TilesetEditor::consume_scroll(event::Scroll const& scroll) -> bool {
	m_render_view.scale.x += scroll.y * m_zoom_speed; // y is adjusted in tick().
	return true;
}

auto TilesetEditor::consume_drop(event::Drop const& drop) -> bool {
	KLIB_ASSERT(!drop.paths.empty());
	auto const& first_path = drop.paths.front();
	try_load_image(first_path);
	return true;
}

void TilesetEditor::tick(kvf::Seconds const /*dt*/) {
	m_render_view.scale.x = std::clamp(m_render_view.scale.x, min_scale_v, max_scale_v);
	m_render_view.scale.y = m_render_view.scale.x;

	if (ImGui::Begin("Info")) {
		ImGui::TextUnformatted(klib::FixedString<256>{"path: {}", m_loaded_path}.c_str());
		auto const size = m_tile_sheet.get_size();
		ImGui::TextUnformatted(klib::FixedString{"{}x{}", size.x, size.y}.c_str());
	}
	ImGui::End();
}

void TilesetEditor::render(Renderer& renderer) const {
	renderer.view = m_render_view;
	m_quad.draw(renderer);
	renderer.view = {};
}

void TilesetEditor::try_load_image(std::string path) {
	auto const bytes = bytes_from_file(path.c_str());
	if (bytes.empty()) {
		// TODO: raise error modal
		return;
	}

	wait_idle();
	if (!m_tile_sheet.load_and_write(bytes)) {
		// TODO: raise error modal
		return;
	}

	m_quad.create(m_tile_sheet.get_size());
	m_loaded_path = std::move(path);
	log::info("loaded image: '{}'", m_loaded_path);
}
} // namespace le::assed
