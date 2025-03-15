#pragma once
#include <imcpp.hpp>
#include <cstdint>
#include <string>

namespace le::assed {
struct SaveModal {
	enum class Result : std::int8_t { None, Save, Cancel };

	auto update() -> Result;

	std::string title{};
	imcpp::InputText uri{};
	bool set_open{};
};
} // namespace le::assed
