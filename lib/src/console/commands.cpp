#include <le2d/console/commands.hpp>
#include <le2d/context.hpp>

namespace le::console {
CmdVsync::CmdVsync(le::IContext& context) : m_context(context) {
	m_args.push_back(klib::args::named_flag(m_list, "l,list", "list supported vsync modes"));
	m_args.push_back(klib::args::named_flag(m_all, "a,all", "print all vsync modes"));
	m_args.push_back(klib::args::positional_optional(m_input, "MODE", "desired mode", &m_input_passed));
}

void CmdVsync::run(le::console::IPrinter& printer) {
	if (m_list) {
		m_list = false;
		list(printer, false);
		return;
	}

	if (m_all) {
		m_all = false;
		list(printer, true);
		return;
	}

	if (!m_input_passed) {
		printer.println(le::vsync_str_v[m_context.get_vsync()]);
		return;
	}

	m_input_passed = false;
	auto const desired = le::parse_vsync(m_input);
	if (!desired) {
		printer.printerr(std::format("unrecognized Vsync: '{}'", m_input));
		return;
	}

	auto const vsync_str = le::vsync_str_v[*desired];
	if (!m_context.set_vsync(*desired)) {
		printer.printerr(std::format("failed to set Vsync to: {}", vsync_str));
		return;
	}

	printer.println(std::format("Vsync set to: {}", vsync_str));
}

void CmdVsync::list(le::console::IPrinter& printer, bool const all) const {
	auto text = std::string{};
	if (all) {
		for (auto const vsync_str : le::vsync_str_v) { std::format_to(std::back_inserter(text), "{}\n", vsync_str); }
	} else {
		for (auto const vsync : m_context.get_supported_vsync()) { std::format_to(std::back_inserter(text), "{}\n", le::vsync_str_v[vsync]); }
	}
	if (!text.empty()) { text.pop_back(); }
	printer.println(text);
}
} // namespace le::console
