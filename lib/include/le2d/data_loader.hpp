#pragma once
#include <klib/base_types.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dj {
class Json;
}

namespace le {
class IDataLoader : public klib::Polymorphic {
  public:
	virtual auto load_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool = 0;
	virtual auto load_spirv(std::vector<std::uint32_t>& out, std::string_view uri) const -> bool = 0;
	virtual auto load_string(std::string& out, std::string_view uri) const -> bool = 0;

	auto load_json(dj::Json& out, std::string_view uri) const -> bool;
	[[nodiscard]] auto get_json_type_name(std::string_view uri) const -> std::string;
};
} // namespace le
