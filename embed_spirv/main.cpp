#include <klib/args/parse.hpp>
#include <klib/file_io.hpp>
#include <array>
#include <cassert>
#include <filesystem>
#include <print>
#include <vector>

namespace embedder {
namespace {
namespace fs = std::filesystem;

struct Config {
	std::string_view ns{};
	std::string_view fn_name{"get_spirv"};
	std::int32_t col_width{120};
};

class Writer {
  public:
	explicit Writer(Config const& config) : m_config(config) {}

	auto write(std::span<std::uint32_t const> code) -> bool {
		if (code.empty()) {
			std::println(stderr, "ERROR: no bytes to write");
			return false;
		}

		includes();
		if (!m_config.ns.empty()) { begin_ns(m_config.ns); }
		begin_ns({});
		begin_array(code.size());
		for (auto const byte : code) { write_next(byte); }
		end_array();
		end_ns({});
		api();
		if (!m_config.ns.empty()) { end_ns(m_config.ns); }

		return true;
	}

  private:
	static void includes() {
		static constexpr std::string_view text_v = R"(#include <array>
#include <cstdint>
#include <span>
)";
		std::println("{}", text_v);
	}

	static void begin_ns(std::string_view const name) {
		if (name.empty()) {
			std::println("namespace {{");
		} else {
			std::println("namespace {} {{", name);
		}
	}

	static void end_ns(std::string_view const name) {
		if (name.empty()) {
			std::println("}} // namespace");
		} else {
			std::println("}} // namespace {}", name);
		}
	}

	void begin_array(std::size_t const size) {
		std::format_to(std::back_inserter(m_buffer), "auto const g_code = std::array<std::uint32_t, {}>{{", size);
		++m_indent;
		flush_buffer();
	}

	void end_array() {
		--m_indent;
		flush_buffer();
		m_buffer.append("};");
		flush_buffer();
	}

	void write_next(std::uint32_t const symbol) {
		m_symbol.clear();
		std::format_to(std::back_inserter(m_symbol), "{}, ", symbol);
		auto const total_size = m_buffer.size() + m_symbol.size();
		if (total_size > std::size_t(m_config.col_width)) { flush_buffer(); }
		m_buffer.append(m_symbol);
		++m_written;
	}

	void flush_buffer() {
		if (m_buffer.empty()) { return; }
		std::println("{}", m_buffer);
		m_buffer.clear();
		indent();
	}

	void indent() {
		for (std::int32_t i = 0; i < m_indent; ++i) { m_buffer += '\t'; }
	}

	void api() {
		std::println();
		std::format_to(std::back_inserter(m_buffer), "auto {}() -> std::span<std::uint32_t const> {{", m_config.fn_name);
		++m_indent;
		flush_buffer();

		m_buffer.append("return g_code;");
		flush_buffer();

		m_indent = 0;
		std::println("}}");
	}

	Config m_config{};

	std::string m_symbol{};
	std::string m_buffer{};
	std::int32_t m_indent{};

	std::uint64_t m_written{};
};

auto run(int argc, char** argv) -> int {
	auto const parse_info = klib::args::ParseInfo{
		.help_text = "embed SPIR-V as C++",
	};
	auto config = Config{};
	auto path = std::string_view{};
	auto const args = std::array{
		klib::args::named_option(config.fn_name, "f,func-name", "name of API function"),
		klib::args::named_option(config.ns, "n,namespace", "name of API namespace"),
		klib::args::named_option(config.col_width, "c,col-width", "column width"),
		klib::args::positional_required(path, "IN"),
	};
	auto const parse_result = klib::args::parse_main(parse_info, args, argc, argv);
	if (parse_result.early_return()) { return parse_result.get_return_code(); }

	auto code = std::vector<std::uint32_t>{};
	if (!klib::read_file_bytes_into(code, path.data())) {
		std::println(stderr, "ERROR: failed to open file: {}", path);
		return EXIT_FAILURE;
	}
	if (code.empty()) {
		std::println(stderr, "ERROR: file is empty: {}", path);
		return EXIT_FAILURE;
	}

	auto const stem = fs::path{path}.stem().generic_string();

	auto writer = Writer{config};
	if (!writer.write(code)) { return EXIT_FAILURE; }

	return EXIT_SUCCESS;
}
} // namespace
} // namespace embedder

auto main(int argc, char** argv) -> int {
	try {
		return embedder::run(argc, argv);
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println(stderr, "PANIC!");
		return EXIT_FAILURE;
	}
}
