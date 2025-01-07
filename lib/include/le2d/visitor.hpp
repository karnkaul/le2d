#pragma once

namespace le {
struct NullVisitor {
	template <typename T>
	constexpr void operator()(T&& /*unused*/) const {}
};

template <typename... Ts>
struct Visitor : Ts... {
	using Ts::operator()...;
};

template <typename... Ts>
struct SubVisitor : Ts..., NullVisitor {
	using NullVisitor::operator();
	using Ts::operator()...;
};

template <typename... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

template <typename... Ts>
SubVisitor(Ts...) -> SubVisitor<Ts...>;
} // namespace le
