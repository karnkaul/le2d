#pragma once
#include <le2d/console/command.hpp>
#include <le2d/vsync.hpp>
#include <vector>

namespace le {
class IContext;

namespace console {
class CmdVsync : public le::console::ICommand {
  public:
	explicit CmdVsync(le::IContext& context);

  private:
	[[nodiscard]] auto get_args() const -> std::span<klib::args::Arg const> final { return m_args; }
	void run(le::console::IPrinter& printer) final;

	void list(le::console::IPrinter& printer, bool all) const;

	le::IContext& m_context;

	std::string_view m_input{};
	bool m_input_passed{};
	bool m_list{};
	bool m_all{};

	std::vector<klib::args::Arg> m_args{};
};
} // namespace console
} // namespace le
