#pragma once
#include <klib/log.hpp>

namespace le::example::log {
constexpr std::string_view tag_v{"le2d::example"};

template <typename... Args>
void error(klib::log::Fmt<Args...> const& fmt, Args&&... args) {
	klib::log::error(tag_v, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void warn(klib::log::Fmt<Args...> const& fmt, Args&&... args) {
	klib::log::warn(tag_v, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void info(klib::log::Fmt<Args...> const& fmt, Args&&... args) {
	klib::log::info(tag_v, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void debug(klib::log::Fmt<Args...> const& fmt, Args&&... args) {
	if constexpr (!klib::log::debug_enabled_v) { return; }
	klib::log::debug(tag_v, fmt, std::forward<Args>(args)...);
}
} // namespace le::example::log
