#pragma once
#include <klib/base_types.hpp>
#include <le2d/tweak/parser.hpp>
#include <functional>

namespace le {
namespace tweak {
/// \returns false if new_value is invalid.
template <typename Type>
using Callback = std::move_only_function<bool(Type const& new_value)>;
} // namespace tweak

/// \brief Interface for concrete Tweakables.
class ITweakable : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto type_name() const -> std::string_view = 0;
	[[nodiscard]] virtual auto as_string() const -> std::string_view = 0;
	virtual auto assign(std::string_view input) -> bool = 0;
};

/// \brief Tweakable wrapper over a Type value.
template <typename Type>
class Tweakable : public ITweakable {
  public:
	using Parser = tweak::Parser<Type>;

	/// \brief Setter hook callback.
	using Callback = tweak::Callback<Type>;

	explicit(false) Tweakable() { set_value({}); }

	template <std::convertible_to<Type> T>
	explicit(false) Tweakable(T t) {
		set_value(std::move(t));
	}

	[[nodiscard]] auto type_name() const -> std::string_view final { return Parser::type_name(); }

	[[nodiscard]] auto as_string() const -> std::string_view final { return m_as_string; }

	/// \brief Parse input into value and assign if successful.
	/// \param input Input to parse.
	/// \returns true if successfully parsed.
	auto assign(std::string_view const input) -> bool final {
		auto value = Type{};
		if (!Parser::parse(input, value)) { return false; }
		return set_value(std::move(value));
	}

	[[nodiscard]] auto get_value() const -> Type const& { return m_value; }
	[[nodiscard]] auto get_value() -> Type& { return m_value; }

	/// \brief Set value and invoke OnChange (if not null).
	/// \param value Value to set.
	auto set_value(Type value) -> bool {
		if (m_on_set && !m_on_set(value)) { return false; }
		m_value = std::move(value);
		m_as_string = Parser::to_string(m_value);
		return true;
	}

	/// \brief Install setter hook callback.
	void on_set(Callback callback) { m_on_set = std::move(callback); }

	explicit(std::same_as<bool, Type>) operator Type const&() const { return m_value; }

  private:
	Type m_value{};
	std::string m_as_string{};
	Callback m_on_set{};
};
} // namespace le
