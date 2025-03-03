#pragma once
#include <klib/concepts.hpp>
#include <le2d/console/command.hpp>
#include <format>

namespace le::console {
template <typename Type>
class Tweak : public ICommand {
  protected:
	[[nodiscard]] virtual auto get() const -> Type = 0;
	virtual void set(Type value) = 0;

  private:
	[[nodiscard]] auto get_args() const -> std::span<klib::args::Arg const> final { return {&m_arg, 1}; }

	void run(IPrinter& printer) final {
		if (!m_input_passed) {
			print_output(printer);
			return;
		}

		m_input_passed = false;
		if (!assign_input()) {
			printer.printerr(std::format("failed to set value to '{}'", m_input));
			return;
		}
	}

	void print_output(IPrinter& printer) const {
		if constexpr (klib::StringyT<Type>) {
			printer.println(get());
		} else {
			printer.println(std::format("{}", get()));
		}
	}

	auto assign_input() -> bool {
		set(m_input);
		return true;
	}

	Type m_input{};
	bool m_input_passed{};
	klib::args::Arg m_arg{klib::args::positional_optional(m_input, "VALUE", {}, &m_input_passed)};
};
} // namespace le::console
