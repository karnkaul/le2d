#pragma once
#include <string>

namespace le {
class Uri {
  public:
	struct Hasher;

	Uri() = default;

	template <std::convertible_to<std::string> Type>
	/*implicit*/ Uri(Type t) : m_str(std::move(t)), m_hash(std::hash<std::string>{}(m_str)) {}

	[[nodiscard]] auto get_string() const -> std::string_view { return m_str; }
	[[nodiscard]] auto get_hash() const -> std::size_t { return m_hash; }

	auto operator==(Uri const& rhs) const -> bool { return m_hash == rhs.m_hash; }

  private:
	std::string m_str{};
	std::size_t m_hash{};
};

struct Uri::Hasher {
	[[nodiscard]] auto operator()(Uri const& uri) const -> std::size_t { return uri.get_hash(); }
};
} // namespace le
