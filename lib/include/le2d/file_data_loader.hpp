#pragma once
#include <le2d/data_loader.hpp>

namespace le {
class FileDataLoader : public IDataLoader {
  public:
	[[nodiscard]] static auto upfind(std::string_view suffix, std::string_view leaf_dir = ".") -> std::string;

	explicit FileDataLoader(std::string_view root_dir = ".");

	auto load_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool final;
	auto load_spirv(std::vector<std::uint32_t>& out, std::string_view uri) const -> bool final;
	auto load_string(std::string& out, std::string_view uri) const -> bool final;

	[[nodiscard]] auto save_bytes(std::span<std::byte const> bytes, std::string_view uri) const -> bool;
	[[nodiscard]] auto save_string(std::string_view text, std::string_view uri) const -> bool;

	[[nodiscard]] auto get_root_dir() const -> std::string_view { return m_root_dir; }
	auto set_root_dir(std::string_view root_dir) -> bool;

	[[nodiscard]] auto get_path(std::string_view uri) const -> std::string;
	[[nodiscard]] auto get_uri(std::string_view path) const -> std::string;

  private:
	template <typename T, typename F>
	auto from_file(T& out, std::string_view uri, F func) const -> bool;

	std::string m_root_dir{};
};
} // namespace le
