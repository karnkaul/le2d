#pragma once
#include <klib/polymorphic.hpp>
#include <le2d/console/printer.hpp>

namespace le::console {
class ICommand : public klib::Polymorphic {
  public:
	virtual void run(IPrinter& printer, std::string_view input) = 0;
};
} // namespace le::console
