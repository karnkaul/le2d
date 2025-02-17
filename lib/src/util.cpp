#include <le2d/util.hpp>
#include <array>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <climits>
#endif

namespace le {
auto util::exe_path() -> std::string {
	static auto ret = std::string{}; // can never change throughout the process existance
	if (!ret.empty()) { return ret; }

#if defined(_WIN32)
	auto buffer = std::array<char, MAX_PATH>{};
	DWORD length = GetModuleFileNameA(nullptr, buffer.data(), buffer.size());
	if (length == 0) { return {}; }
	ret = std::string{buffer.data(), length};
#elif defined(__linux__)
	auto buffer = std::array<char, PATH_MAX>{};
	ssize_t length = ::readlink("/proc/self/exe", buffer.data(), buffer.size());
	if (length == -1) { return {}; }
	ret = std::string{buffer.data(), std::size_t(length)};
#endif

	return ret;
}
} // namespace le
