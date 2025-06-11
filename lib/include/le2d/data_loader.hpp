#pragma once
#include <klib/base_types.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dj {
class Json;
} // namespace dj

namespace le {
/// \brief Interface for loading bytes from a data source (usually the filesystem).
class IDataLoader : public klib::Polymorphic {
  public:
	/// \param out Destination buffer.
	/// \param uri URI to load from.
	/// \returns true if successfully loaded.
	virtual auto load_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool = 0;
	/// \param out Destination buffer.
	/// \param uri URI to load from.
	/// \returns true if successfully loaded.
	virtual auto load_spirv(std::vector<std::uint32_t>& out, std::string_view uri) const -> bool = 0;
	/// \param out Destination buffer.
	/// \param uri URI to load from.
	/// \returns true if successfully loaded.
	virtual auto load_string(std::string& out, std::string_view uri) const -> bool = 0;

	/// \param out Destination JSON.
	/// \param uri URI to load from.
	/// \returns true if successfully loaded.
	auto load_json(dj::Json& out, std::string_view uri) const -> bool;
	/// \param uri URI to JSON.
	/// \returns JSON type name (if present).
	[[nodiscard]] auto get_json_type_name(std::string_view uri) const -> std::string;
};
} // namespace le
