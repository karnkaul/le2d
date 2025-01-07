#pragma once
#include <stdexcept>

namespace le {
class Error : public std::runtime_error {
	using std::runtime_error::runtime_error;
};
} // namespace le
