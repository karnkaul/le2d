#pragma once
#include <klib/args/arg.hpp>
#include <klib/args/printer.hpp>
#include <klib/polymorphic.hpp>

namespace le::console {
using IPrinter = klib::args::IPrinter;

class ICommand : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_args() const -> std::span<klib::args::Arg const> { return {}; }
	[[nodiscard]] virtual auto get_help_text() const -> std::string_view { return {}; }
	virtual void run(IPrinter& printer) = 0;
};
} // namespace le::console
