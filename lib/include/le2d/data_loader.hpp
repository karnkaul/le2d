#pragma once
#include <klib/base_types.hpp>
#include <le2d/uri.hpp>
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
	virtual auto load_bytes(std::vector<std::byte>& out, Uri const& uri) const -> bool = 0;
	virtual auto load_spirv(std::vector<std::uint32_t>& out, Uri const& uri) const -> bool = 0;
	virtual auto load_string(std::string& out, Uri const& uri) const -> bool = 0;

	auto load_json(dj::Json& out, Uri const& uri) const -> bool;
};
} // namespace le
