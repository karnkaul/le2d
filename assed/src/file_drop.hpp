#pragma once
#include <klib/enum_ops.hpp>
#include <le2d/file_data_loader.hpp>
#include <cstdint>

namespace le::assed {
struct FileDrop {
	enum class Type : std::int8_t {
		None = 0,
		Unknown = 1 << 0,
		Json = 1 << 1,
		Image = 1 << 2,
		Font = 1 << 3,
		Directory = 1 << 4,
	};

	[[nodiscard]] static auto create(FileDataLoader const& loader, std::string_view path) -> FileDrop;

	explicit operator bool() const { return !uri.get_string().empty(); }

	Type type{};
	Uri uri{};
	std::string extension{};
	std::string json_type{};
};
} // namespace le::assed

namespace klib {
template <>
inline constexpr auto enable_enum_ops_v<le::assed::FileDrop::Type> = true;
} // namespace klib
