#include <applet/applet.hpp>
#include <kvf/util.hpp>
#include <le2d/input/dispatch.hpp>

namespace le::assed {
namespace {
template <typename T, typename F>
auto from_file(klib::CString const path, F func) -> T {
	auto ret = T{};
	if (!func(ret, path)) {
		log::error("failed to load: '{}'", path.as_view());
		return {};
	}
	return ret;
}
} // namespace

Applet::Applet(gsl::not_null<ServiceLocator const*> services) : m_services(services) { services->get<input::Dispatch>().attach(this); }

auto Applet::bytes_from_file(klib::CString const path) -> std::vector<std::byte> {
	return from_file<std::vector<std::byte>>(path, &kvf::util::bytes_from_file);
}

auto Applet::string_from_file(klib::CString const path) -> std::string { return from_file<std::string>(path, &kvf::util::string_from_file); }

auto Applet::json_from_file(klib::CString const path) -> dj::Json {
	auto const text = string_from_file(path);
	return dj::Json::parse(text);
}
} // namespace le::assed
