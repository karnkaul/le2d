#include "le2d/file_data_loader.hpp"
#include "klib/file_io.hpp"
#include <log.hpp>
#include <filesystem>

namespace le {
namespace fs = std::filesystem;

auto FileDataLoader::upfind(std::string_view const suffix, std::string_view leaf_dir) -> std::string {
	for (auto dir = fs::absolute(leaf_dir); !dir.empty() && dir.parent_path() != dir; dir = dir.parent_path()) {
		auto const ret = dir / suffix;
		if (fs::exists(ret)) { return ret.generic_string(); }
	}
	log.warn("FileDataLoader: failed to upfind path with suffix: '{}' starting at leaf dir: '{}'", suffix, leaf_dir);
	return {};
}

FileDataLoader::FileDataLoader(std::string_view const root_dir) { set_root_dir(root_dir); }

auto FileDataLoader::try_load_bytes(std::vector<std::byte>& out, std::string_view const uri) const -> bool { return try_load_to(out, uri); }

auto FileDataLoader::try_load_spirv(std::vector<std::uint32_t>& out, std::string_view const uri) const -> bool { return try_load_to(out, uri); }

auto FileDataLoader::try_load_string(std::string& out, std::string_view const uri) const -> bool { return try_load_to(out, uri); }

auto FileDataLoader::save_bytes(std::span<std::byte const> bytes, std::string_view const uri) const -> bool {
	if (uri.empty()) { return false; }
	return klib::write_bytes_to_file(bytes, get_path(uri));
}

auto FileDataLoader::save_string(std::string_view text, std::string_view const uri) const -> bool {
	if (uri.empty()) { return false; }
	return klib::write_to_file(text, get_path(uri));
}

auto FileDataLoader::set_root_dir(std::string_view root_dir) -> bool {
	if (root_dir.empty()) { root_dir = "."; }
	if (!fs::is_directory(root_dir)) {
		log.warn("FileDataLoader: failed to set root directory: '{}'", root_dir);
		return false;
	}
	log.info("FileDataLoader: root directory set to: '{}'", root_dir);
	m_root_dir = fs::absolute(root_dir).generic_string();
	return true;
}

auto FileDataLoader::get_path(std::string_view const uri) const -> std::string { return (fs::path{m_root_dir} / uri).generic_string(); }

auto FileDataLoader::get_uri(std::string_view const path) const -> std::string {
	if (path.empty()) { return {}; }
	auto ret = fs::path{path}.lexically_relative(m_root_dir).generic_string();
	if (ret.starts_with(".")) { return {}; }
	return ret;
}

template <typename T>
auto FileDataLoader::try_load_to(T& out, std::string_view const uri) const -> bool {
	auto const path = get_path(uri);
	if constexpr (sizeof(typename T::value_type) > 1) {
		return klib::copy_file_bytes_to(out, path);
	} else {
		return klib::read_file_bytes_to(out, path);
	}
}
} // namespace le
