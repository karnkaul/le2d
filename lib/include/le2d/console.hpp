#pragma once
#include <kvf/color.hpp>
#include <le2d/drawables/input_text.hpp>
#include <cstdint>
#include <functional>
#include <gsl/pointers>
#include <memory>
#include <string_view>

namespace le {
class Font;
}

namespace le::console {
class Terminal;

[[nodiscard]] auto to_f32(std::string_view str, float fallback = {}) -> float;
[[nodiscard]] auto to_i64(std::string_view str, std::int64_t fallback = {}) -> std::int64_t;
[[nodiscard]] auto to_u64(std::string_view str, std::uint64_t fallback = {}) -> std::uint64_t;

class Stream : public klib::Polymorphic {
  public:
	virtual auto next_arg(std::string_view& out) -> bool = 0;
	virtual auto next_arg() -> std::string_view = 0;

	virtual void println(std::string_view line) = 0;
	virtual void printerr(std::string_view line) = 0;
};
using Command = std::move_only_function<void(Stream& stream)>;

struct TerminalCreateInfo {
	glm::vec2 framebuffer_size;
	InputTextParams input_text{};
	std::size_t buffer_size{32};
	std::size_t history_size{32};
	float line_spacing{1.1f};
	char caret{'$'};
	struct {
		float height{2.0f};
		kvf::Color color{kvf::white_v};
	} separator{};
	float y_speed{4000.0f};
	float x_pad{15.0f};
	struct {
		kvf::Color input{kvf::yellow_v};
		kvf::Color output{0xccccccff};
		kvf::Color error{kvf::red_v};
	} text_colors{};
};

class Terminal : public IDrawable {
  public:
	using CreateInfo = TerminalCreateInfo;

	explicit Terminal(gsl::not_null<Font*> font, CreateInfo const& info);

	void add_command(std::string_view name, Command command);

	[[nodiscard]] auto is_active() const -> bool;

	[[nodiscard]] auto get_opacity() const -> float;
	void set_opacity(float value);

	void on_resize(event::FramebufferResize size);
	void on_key(event::Key const& key);
	void on_codepoint(event::Codepoint codepoint);
	void tick(kvf::Seconds dt);
	void draw(Renderer& renderer) const final;

  private:
	struct Impl;
	struct Deleter {
		void operator()(Impl* ptr) const noexcept;
	};
	std::unique_ptr<Impl, Deleter> m_impl{};

	friend class Stream;
};
} // namespace le::console
