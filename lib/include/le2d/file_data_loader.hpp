#pragma once
#include <le2d/data_loader.hpp>

namespace le {
/// \brief Concrete IDataLoader that uses the filesystem.
class FileDataLoader : public IDataLoader {
  public:
	/// \brief Locate a directory by cycling upwards every iteration.
	/// \param suffix Pattern to search for (eg "assets" or "resources/data").
	/// \param leaf_dir Starting directory (usually working dir or exe dir).
	[[nodiscard]] static auto upfind(std::string_view suffix, std::string_view leaf_dir = ".") -> std::string;

	/// \param root_dir Mount point. Will be prefixed to URIs for loads.
	explicit FileDataLoader(std::string_view root_dir = ".");

	auto try_load_bytes(std::vector<std::byte>& out, std::string_view uri) const -> bool final;
	auto try_load_spirv(std::vector<std::uint32_t>& out, std::string_view uri) const -> bool final;
	auto try_load_string(std::string& out, std::string_view uri) const -> bool final;

	/// \param bytes Bytes to save to file.
	/// \param uri URI to save file at.
	[[nodiscard]] auto save_bytes(std::span<std::byte const> bytes, std::string_view uri) const -> bool;
	/// \param text Text to save to file.
	/// \param uri URI to save file at.
	[[nodiscard]] auto save_string(std::string_view text, std::string_view uri) const -> bool;

	/// \returns Current root dir.
	[[nodiscard]] auto get_root_dir() const -> std::string_view { return m_root_dir; }
	/// \param root_dir Mount point. Will be prefixed to URIs for loads.
	/// \returns true if successfully mounted.
	auto set_root_dir(std::string_view root_dir) -> bool;

	/// \param uri URI to get full path for.
	/// \returns Full path (whether it exists or not).
	[[nodiscard]] auto get_path(std::string_view uri) const -> std::string;
	/// \param path path to get URI for.
	/// \returns URI corresponding to path, if within root dir.
	[[nodiscard]] auto get_uri(std::string_view path) const -> std::string;

  private:
	template <typename T, typename F>
	auto from_file(T& out, std::string_view uri, F func) const -> bool;

	std::string m_root_dir{};
};
} // namespace le
