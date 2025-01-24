#pragma once
#include <kvf/color.hpp>
#include <kvf/time.hpp>
#include <le2d/drawables/drawable.hpp>
#include <le2d/event.hpp>
#include <le2d/font.hpp>
#include <functional>
#include <gsl/pointers>
#include <memory>
#include <string_view>

namespace le {
class Font;
}

namespace le::console {
class Terminal;

class Stream : public klib::Polymorphic {
  public:
	virtual auto next_arg(std::string_view& out) -> bool = 0;
	virtual auto next_arg() -> std::string_view = 0;

	virtual void println(std::string_view line) = 0;
	virtual void printerr(std::string_view line) = 0;
};
using Command = std::move_only_function<void(Stream& stream)>;

struct TerminalCreateInfo {
	struct {
		std::size_t history{32};
		std::size_t buffer{32};
	} storage{};

	struct {
		TextHeight text_height{TextHeight::Default};
		float line_spacing{1.1f};
		float separator_height{2.0f};
		float x_pad{15.0f};
		char caret{'>'};
		char cursor{'|'};
	} style{};

	struct {
		float slide_speed{4000.0f};
		float scroll_speed{30.0f};
		kvf::Seconds blink_period{1s};
	} motion{};

	struct {
		kvf::Color cursor{kvf::magenta_v};
		kvf::Color separator{kvf::white_v};
		kvf::Color input{kvf::yellow_v};
		kvf::Color output{0xccccccff};
		kvf::Color error{kvf::red_v};
	} colors{};
};

class Terminal : public IDrawable {
  public:
	using CreateInfo = TerminalCreateInfo;

	explicit Terminal(gsl::not_null<Font*> font, glm::vec2 framebuffer_size, CreateInfo const& info = {});

	void add_command(std::string_view name, Command command);

	[[nodiscard]] auto is_active() const -> bool;

	[[nodiscard]] auto get_background() const -> kvf::Color;
	void set_background(kvf::Color color);

	void resize(glm::ivec2 framebuffer_size);

	void on_key(event::Key const& key);
	void on_codepoint(event::Codepoint codepoint);
	void on_scroll(event::Scroll scroll);

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
