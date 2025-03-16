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
} // namespace

Applet::Applet(gsl::not_null<ServiceLocator const*> services) : m_services(services) {
	services->get<input::Dispatch>().attach(this);
	m_save_modal.root_dir = services->get<FileDataLoader>().get_root_dir();
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
