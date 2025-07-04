#include <kvf/util.hpp>
#include <le2d/file_data_loader.hpp>
#include <log.hpp>
#include <filesystem>
#include <fstream>

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

auto FileDataLoader::try_load_bytes(std::vector<std::byte>& out, std::string_view const uri) const -> bool {
	return from_file(out, uri, &kvf::util::bytes_from_file);
}

auto FileDataLoader::try_load_spirv(std::vector<std::uint32_t>& out, std::string_view const uri) const -> bool {
	return from_file(out, uri, &kvf::util::spirv_from_file);
}

auto FileDataLoader::try_load_string(std::string& out, std::string_view const uri) const -> bool { return from_file(out, uri, &kvf::util::string_from_file); }

auto FileDataLoader::save_bytes(std::span<std::byte const> bytes, std::string_view const uri) const -> bool {
	if (uri.empty()) { return false; }
	auto file = std::ofstream{get_path(uri), std::ios::binary};
	if (!file.is_open()) { return false; }
	if (bytes.empty()) { return true; }
	auto const* data = static_cast<void const*>(bytes.data());
	return !!file.write(static_cast<char const*>(data), std::streamsize(bytes.size()));
}

auto FileDataLoader::save_string(std::string_view text, std::string_view const uri) const -> bool {
	auto const* data = static_cast<void const*>(text.data());
	return save_bytes(std::span{static_cast<std::byte const*>(data), text.size()}, uri);
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

template <typename T, typename F>
auto FileDataLoader::from_file(T& out, std::string_view const uri, F func) const -> bool {
	if (!func(out, get_path(uri).c_str())) {
		return false;
		log.warn("FileDataLoader: failed to load: '{}'", uri);
	}
	return true;
}
} // namespace le
