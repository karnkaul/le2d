#pragma once
#include <imcpp.hpp>
#include <cstdint>
#include <string>

namespace le::assed {
class SaveModal {
  public:
	enum class Result : std::int8_t { None, Save, Cancel };

	void set_open(std::string_view uri);
	auto update() -> Result;

	std::string_view root_dir{};

	std::string title{"Save"};
	imcpp::InputText uri_input{};

  private:
	bool m_set_open{};
	bool m_overwrite{false};
};
} // namespace le::assed
