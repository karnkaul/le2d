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

Applet::Applet(gsl::not_null<ServiceLocator const*> services) : m_services(services) { services->get<input::Dispatch>().attach(this); }

auto Applet::load_bytes(Uri const& uri) const -> std::vector<std::byte> {
	return load<std::vector<std::byte>>(get_data_loader(), uri, &IDataLoader::load_bytes);
}

auto Applet::load_string(Uri const& uri) const -> std::string { return load<std::string>(get_data_loader(), uri, &IDataLoader::load_string); }

auto Applet::load_json(Uri const& uri) const -> dj::Json { return load<dj::Json>(get_data_loader(), uri, &IDataLoader::load_json); }

void Applet::raise_error(std::string message, std::string title) {
	m_error_modal = ErrorModal{.title = std::move(title), .message = std::move(message)};
	ImGui::OpenPopup(m_error_modal.title.c_str());
}

void Applet::do_tick(kvf::Seconds const dt) {
	tick(dt);
	m_error_modal();
}

void Applet::ErrorModal::operator()() const {
	if (imcpp::begin_modal(title.c_str())) {
		ImGui::TextUnformatted(message.c_str());
		ImGui::Separator();
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}
} // namespace le::assed
