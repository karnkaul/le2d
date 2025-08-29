#include <applet/font_viewer.hpp>
#include <klib/assert.hpp>
#include <le2d/input/dispatch.hpp>
#include <algorithm>

namespace le::assed {
FontViewer::FontViewer(gsl::not_null<ServiceLocator const*> services)
	: Applet(services), m_font(services->get<Context>().get_resource_factory().create_font()) {
	m_quad.create();
	m_drop_types = FileDrop::Type::Font;
}

auto FontViewer::consume_key(event::Key const& key) -> bool {
	if (!m_input_text) { return false; }
	auto const was_interactive = m_input_text->is_interactive();
	m_input_text->on_key(key);
	return was_interactive || m_input_text->is_interactive();
}

auto FontViewer::consume_codepoint(event::Codepoint const codepoint) -> bool {
	if (!m_input_text) { return false; }
	m_input_text->on_codepoint(codepoint);
	return m_input_text->is_interactive();
}

void FontViewer::tick(kvf::Seconds const dt) {
	if (m_input_text) { m_input_text->tick(dt); }

	inspect();
}

void FontViewer::render(IRenderer& renderer) const {
	renderer.view = m_render_view;
	if (m_display == Display::InputText && m_input_text) {
		m_input_text->draw(renderer);
	} else {
		m_quad.draw(renderer);
	}
	renderer.view = {};
}

void FontViewer::on_drop(FileDrop const& drop) {
	KLIB_ASSERT(drop.type == FileDrop::Type::Font);
	try_load_font(drop.uri);
}

void FontViewer::inspect() {
	if (ImGui::Begin("Info")) {
		if (ImGui::TreeNode("colors")) {
			imcpp::color_edit("background", clear_color);
			if (imcpp::color_edit("text", m_quad.tint) && m_input_text) { m_input_text->tint = m_quad.tint; }
			ImGui::TreePop();
		}
		if (m_font->is_ready()) {
			auto text_height = int(m_text_height);
			if (ImGui::DragInt("height", &text_height, 5.0f, int(TextHeight::Min), int(TextHeight::Max))) { set_text_height(TextHeight(text_height)); }
		}

		ImGui::Separator();
		inspect_display();
		if (m_input_text && m_display == Display::InputText) { inspect_input_text(); }
	}
	ImGui::End();
}

void FontViewer::inspect_display() {
	static constexpr auto display_str_v = klib::EnumArray<Display, klib::CString>{
		"Atlas",
		"InputText",
	};
	if (m_input_text && ImGui::BeginCombo("Display", display_str_v[m_display].c_str())) {
		for (auto d = Display{}; d < Display::COUNT_; d = Display(int(d) + 1)) {
			if (ImGui::Selectable(display_str_v[d].c_str())) { set_display(d); }
		}
		ImGui::EndCombo();
	}
}

void FontViewer::inspect_input_text() {
	ImGui::DragFloat2("position", &m_input_text->transform.position.x);
	auto interactive = m_input_text->is_interactive();
	if (ImGui::Checkbox("interactive", &interactive)) { m_input_text->set_interactive(interactive); }
}

void FontViewer::try_load_font(Uri const& uri) {
	auto const& asset_loader = get_asset_loader();
	auto font = asset_loader.load<IFont>(uri.get_string());
	if (!font) {
		raise_error(std::format("Failed to load font: '{}'", uri.get_string()));
		return;
	}

	wait_idle();
	m_font = std::move(font);
	set_text_height(TextHeight::Default);

	set_title(uri.get_string());
	log.info("loaded Font: '{}'", uri.get_string());
}

void FontViewer::set_text_height(TextHeight const height) {
	KLIB_ASSERT(m_font->is_ready());
	m_text_height = std::clamp(height, TextHeight::Min, TextHeight::Max);
	m_atlas = &m_font->get_atlas(m_text_height);
	m_quad.texture = &m_atlas->get_texture();
	m_quad.create(m_quad.texture->get_size());
	create_input_text();
}

void FontViewer::create_input_text() {
	auto text = std::string{};
	auto position = glm::vec2{};
	if (m_input_text) {
		text = std::string{m_input_text->get_string()};
		position = m_input_text->transform.position;
	}
	auto const params = InputTextParams{
		.height = m_text_height,
	};
	m_input_text.emplace(m_font.get(), params);
	m_input_text->tint = m_quad.tint;
	m_input_text->transform.position = position;
	if (!text.empty()) { m_input_text->set_string(std::move(text)); }
}

void FontViewer::set_display(Display const display) {
	if (display == m_display) { return; }
	m_display = display;
	m_input_text->set_interactive(m_display == Display::InputText);
}
} // namespace le::assed
