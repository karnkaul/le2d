#pragma once
#include <klib/polymorphic.hpp>
#include <string_view>

namespace le::console {
class IPrinter : public klib::Polymorphic {
  public:
	virtual void println(std::string_view text) = 0;
	virtual void printerr(std::string_view text) = 0;
};
} // namespace le::console
