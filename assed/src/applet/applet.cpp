#include <applet/applet.hpp>
#include <le2d/input/dispatch.hpp>
#include <functional>

namespace le::assed {
namespace {
template <typename T, typename F>
auto load(IDataLoader const& data_loader, Uri const& uri, F func) -> T {
	auto ret = T{};
	if (!std::invoke(func, &data_loader, ret, uri)) { return {}; }
	return ret;
}

constexpr auto min_scale_v{0.1f};
constexpr auto max_scale_v{10.0f};
} // namespace

Applet::Applet(gsl::not_null<ServiceLocator const*> services) : m_services(services) {
	services->get<input::Dispatch>().attach(this);
	m_save_modal.root_dir = services->get<FileDataLoader>().get_root_dir();
}

auto Applet::consume_scroll(event::Scroll const& scroll) -> bool {
	if (m_zoom_speed > 0.0f) {
		m_render_view.scale.x += scroll.y * m_zoom_speed;

		m_render_view.scale.x = std::clamp(m_render_view.scale.x, min_scale_v, max_scale_v);
		m_render_view.scale.y = m_render_view.scale.x;
	}

	return true;
}

auto Applet::consume_drop(event::Drop const& drop) -> bool {
	KLIB_ASSERT(!drop.paths.empty());
	auto const& first_path = drop.paths.front();
	auto const file_drop = FileDrop::create(get_data_loader(), first_path);
	if (!file_drop) {
		raise_error(std::format("Path is not in asset directory\n{}", first_path));
		return true;
	}

	if ((file_drop.type & m_drop_types) != file_drop.type) {
		raise_error(std::format("Unsupported file type: {}", file_drop.uri.get_string()));
		return true;
	}

	if (file_drop.type == FileDrop::Type::Json) {
		if (std::ranges::find(m_json_types, file_drop.json_type) == m_json_types.end()) {
			raise_error(std::format("Unsupported asset type: {}", file_drop.json_type));
			return true;
		}
	}

	on_drop(file_drop);
	return true;
}

auto Applet::load_bytes(Uri const& uri) const -> std::vector<std::byte> {
	return load<std::vector<std::byte>>(get_data_loader(), uri, &IDataLoader::load_bytes);
}

auto Applet::load_string(Uri const& uri) const -> std::string { return load<std::string>(get_data_loader(), uri, &IDataLoader::load_string); }

auto Applet::load_json(Uri const& uri) const -> dj::Json { return load<dj::Json>(get_data_loader(), uri, &IDataLoader::load_json); }

void Applet::raise_dialog(std::string message, std::string title) {
	m_dialog = Dialog{.title = std::move(title), .message = std::move(message)};
	ImGui::OpenPopup(m_dialog.title.c_str());
}

void Applet::set_title(std::string_view const uri) const {
	auto const name = get_name();
	if (uri.empty()) {
		get_context().get_render_window().set_title(name);
		return;
	}
	auto const suffix = m_unsaved ? std::string_view{"*"} : std::string_view{};
	auto const title = std::format("{} - {}{}", name.as_view(), uri, suffix);
	get_context().get_render_window().set_title(title.c_str());
}

void Applet::do_setup() {
	set_title();
	setup();
}

void Applet::do_tick(kvf::Seconds const dt) {
	tick(dt);

	m_dialog();
	if (m_open_confirm_exit) {
		m_open_confirm_exit = false;
		ImGui::OpenPopup(ConfirmExitDialog::label_v.c_str());
	}
	auto const result = m_confirm_exit();
	if (result == ConfirmExitDialog::Result::Quit) { get_context().shutdown(); }
}

auto Applet::try_exit() -> bool {
	if (!m_unsaved) { return true; }
	m_open_confirm_exit = true;
	return false;
}

void Applet::Dialog::operator()() const {
	if (imcpp::begin_modal(title.c_str())) {
		ImGui::TextUnformatted(message.c_str());
		ImGui::Separator();
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

auto Applet::ConfirmExitDialog::operator()() const -> Result {
	auto ret = Result::None;
	if (imcpp::begin_modal(label_v.c_str())) {
		ImGui::TextUnformatted("There are unsaved changes. Exit?");
		ImGui::Separator();
		if (ImGui::Button("Exit")) {
			ret = Result::Quit;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ret = Result::Cancel;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	return ret;
}
} // namespace le::assed
