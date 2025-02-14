#pragma once
#include <klib/args/arg.hpp>
#include <klib/args/printer.hpp>
#include <kvf/color.hpp>
#include <kvf/time.hpp>
#include <le2d/drawable/drawable.hpp>
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

using Printer = klib::args::IPrinter;

class Command : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_name() const -> std::string_view = 0;
	[[nodiscard]] virtual auto get_description() const -> std::string_view { return {}; }
	[[nodiscard]] virtual auto get_args() const -> std::span<klib::args::Arg const> { return {}; }

	virtual void execute(Printer& printer) = 0;
};

struct TerminalCreateInfo {
	struct {
		std::size_t history{32};
		std::size_t buffer{128};
	} storage{};

	struct {
		TextHeight text_height{TextHeight{20}};
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

class Terminal : public IDrawable, public Printer {
  public:
	using CreateInfo = TerminalCreateInfo;

	explicit Terminal(gsl::not_null<Font*> font, glm::vec2 framebuffer_size, CreateInfo const& info = {});

	void add_command(std::unique_ptr<Command> command);
	void add_command(std::string_view name, std::string_view description, std::move_only_function<void(Printer&)> command);

	[[nodiscard]] auto is_active() const -> bool;

	void println(std::string_view text) final;
	void printerr(std::string_view text) final;

	[[nodiscard]] auto get_background() const -> kvf::Color;
	void set_background(kvf::Color color);

	void handle_events(std::span<Event const> events, bool* activated = nullptr);

	void tick(glm::vec2 framebuffer_size, kvf::Seconds dt);
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
