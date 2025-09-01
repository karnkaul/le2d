#pragma once
#include <glm/vec2.hpp>
#include <charconv>
#include <format>
#include <string>

namespace le::tweak {
template <typename Type>
concept IntegerT = std::same_as<Type, std::int64_t> || std::same_as<Type, std::int32_t>;

template <typename Type>
concept UnsignedT = std::same_as<Type, std::uint64_t> || std::same_as<Type, std::uint32_t>;

template <typename Type>
concept FloatT = std::same_as<Type, float> || std::same_as<Type, double>;

template <typename Type>
concept NumberT = IntegerT<Type> || UnsignedT<Type> || FloatT<Type>;

/// \brief Customization point. Specialize this type for custom Tweakable template types.
template <typename Type>
struct Parser;

template <>
struct Parser<bool> {
	[[nodiscard]] static constexpr auto type_name() -> std::string_view { return "bool"; }
	[[nodiscard]] static auto parse(std::string_view in, bool& out) -> bool;
	[[nodiscard]] static auto to_string(bool const value) -> std::string { return value ? "true" : "false"; }
};

template <>
struct Parser<std::string> {
	[[nodiscard]] static constexpr auto type_name() -> std::string_view { return "string"; }
	[[nodiscard]] static auto parse(std::string_view in, std::string& out) -> bool;
	[[nodiscard]] static auto to_string(std::string_view const value) -> std::string { return std::string{value}; }
};

template <NumberT Type>
struct Parser<Type> {
	[[nodiscard]] static constexpr auto type_name() -> std::string_view {
		if constexpr (FloatT<Type>) {
			return "float";
		} else if constexpr (UnsignedT<Type>) {
			return "unsigned";
		} else {
			return "integer";
		}
	}

	[[nodiscard]] static auto parse(std::string_view const in, Type& out) -> bool {
		char const* end = in.data() + in.size();
		auto const [ptr, ec] = std::from_chars(in.data(), end, out);
		return ec == std::errc{} && ptr == end;
	}

	[[nodiscard]] static auto to_string(Type const value) -> std::string { return std::format("{}", value); }
};

template <NumberT Type>
struct Parser<glm::tvec2<Type>> {
	[[nodiscard]] static constexpr auto type_name() -> std::string_view {
		if constexpr (UnsignedT<Type>) {
			return "uvec2";
		} else if constexpr (IntegerT<Type>) {
			return "ivec2";
		} else {
			return "vec2";
		}
	}

	[[nodiscard]] static auto parse(std::string_view const in, glm::tvec2<Type>& out) -> bool {
		auto const i = in.find('x');
		if (i == std::string_view::npos) { return false; }
		if (!Parser<Type>::parse(in.substr(0, i), out.x)) { return false; }
		if (!Parser<Type>::parse(in.substr(i + 1), out.y)) { return false; }
		return true;
	}

	[[nodiscard]] static auto to_string(glm::tvec2<Type> const value) -> std::string { return std::format("{}x{}", value.x, value.y); }
};
} // namespace le::tweak
