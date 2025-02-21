#include <GLFW/glfw3.h>
#include <klib/args/parse.hpp>
#include <klib/assert.hpp>
#include <klib/concepts.hpp>
#include <klib/str_to_num.hpp>
#include <kvf/util.hpp>
#include <le2d/console.hpp>
#include <le2d/drawable/input_text.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/text/text_buffer.hpp>
#include <algorithm>
#include <ranges>

#include <print>

namespace le::console {
namespace {
struct Caret {
	TextGeometry geometry{};
	Texture const* texture{};
	RenderInstance instance{};
	float text_x{};

	void create(FontAtlas& atlas, char const c) {
		auto const text = std::format("{} ", c);
		auto layouts = std::vector<kvf::ttf::GlyphLayout>{};
		layouts.reserve(2);
		text_x = atlas.push_layouts(layouts, text).x;
		geometry.append_glyphs(layouts);
		texture = &atlas.get_texture();
	}

	void draw(Renderer& renderer) const {
		auto const primitive = geometry.to_primitive(*texture);
		renderer.draw(primitive, {&instance, 1});
	}
};

struct Line {
	std::string text{};
	kvf::Color color{};
};

struct Buffer {
	struct Printer;

	explicit Buffer(FontAtlas& atlas, std::size_t const limit, float n_line_spacing) : m_text_buffer(&atlas, limit, n_line_spacing) {}

	void push(std::string text, kvf::Color color) { m_text_buffer.push_front(std::move(text), color); }
	void push(std::span<std::string> lines, kvf::Color color) { m_text_buffer.push_front(lines, color); }

	[[nodiscard]] auto get_height() const -> float { return m_text_buffer.get_size().y; }

	void draw(Renderer& renderer) const {
		auto const primitive = m_text_buffer.to_primitive();
		auto const instance = RenderInstance{.transform = {.position = position}};
		renderer.draw(primitive, {&instance, 1});
	}

	glm::vec2 position{};

  private:
	TextBuffer m_text_buffer;
};

struct Buffer::Printer {
	explicit Printer(Buffer& buffer) : m_buffer(buffer) {}

	void print(std::string_view const text, kvf::Color const color) const {
		auto lines = std::vector<std::string>{};
		for (auto const view : std::views::split(text, std::string_view{"\n"})) { lines.emplace_back(std::string_view{view}); }
		m_buffer.push(lines, color);
	}

  private:
	Buffer& m_buffer;
};

constexpr auto longest_match(std::span<std::string_view const> candidates, std::string_view const current) {
	if (candidates.size() < 2) { return current; }
	auto const source = candidates.front();
	auto ret = std::string_view{source}.substr(0, current.size());
	auto const is_match = [&](std::size_t const index) {
		for (std::size_t i = 1; i < candidates.size(); ++i) {
			auto const candidate = candidates[i];
			if (index >= candidate.size()) { return false; }
			if (candidate[index] != source[index]) { return false; }
		}
		return true;
	};
	for (auto index = ret.size(); index < source.size(); ++index) {
		if (!is_match(index)) { break; }
		ret = source.substr(0, index + 1);
	}
	return ret;
}

template <klib::NumberT T>
auto to_num(std::string_view str, T const fallback) -> T {
	auto ret = T{};
	auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), ret);
	if (ec != std::errc{}) { return fallback; }
	return ret;
}
} // namespace

struct Terminal::Impl : Printer {
	static constexpr auto to_input_text_params(CreateInfo const& in) {
		return InputTextParams{
			.height = in.style.text_height,
			.cursor_symbol = in.style.cursor,
			.cursor_color = in.colors.cursor,
			.blink_period = in.motion.blink_period,
		};
	}

	explicit Impl(Font& font, glm::vec2 const framebuffer_size, CreateInfo const& info)
		: m_info(info), m_framebuffer_size(framebuffer_size), m_input(&font, to_input_text_params(info)),
		  m_buffer(font.get_atlas(m_info.style.text_height), m_info.storage.buffer, m_info.style.line_spacing) {
		setup(font);
		add_command(std::make_unique<Builtin>(*this));
		add_command(std::make_unique<Help>(*this));
	}

	void toggle_active() {
		m_active = !m_active;
		m_input.set_interactive(m_active);
	}

	[[nodiscard]] auto is_active() const -> bool { return m_active; }

	void add_command(std::unique_ptr<Command> command) {
		if (!command) { return; }
		auto const name = command->get_name();
		if (name.empty()) { return; }
		m_args.push_back(klib::args::command(command->get_args(), name, command->get_description()));
		auto* p_command = command.get();
		m_commands.push_back(std::move(command));
		m_cmd_map.insert_or_assign(name, p_command);
	}

	void add_command(std::string_view const name, std::string_view const description, std::move_only_function<void(Printer&)> command) {
		if (name.empty() || !command) { return; }

		struct Closure : Command {
			explicit Closure(std::string_view const name, std::string_view const description, std::move_only_function<void(Printer&)> func)
				: m_name(name), m_description(description), m_func(std::move(func)) {}

		  private:
			[[nodiscard]] auto get_name() const -> std::string_view final { return m_name; }
			[[nodiscard]] auto get_description() const -> std::string_view final { return m_description; }
			void execute(Printer& printer) final { m_func(printer); }

			std::string_view m_name;
			std::string_view m_description;
			std::move_only_function<void(Printer&)> m_func;
		};

		add_command(std::make_unique<Closure>(name, description, std::move(command)));
	}

	[[nodiscard]] auto get_background() const -> kvf::Color { return m_background.tint; }

	void set_background(kvf::Color const color) { m_background.tint = color; }

	void println(std::string_view const text) final { Buffer::Printer{m_buffer}.print(text, m_info.colors.output); }
	void printerr(std::string_view const text) final { Buffer::Printer{m_buffer}.print(text, m_info.colors.error); }

	void handle_events(std::span<Event const> events, bool* activated) {
		auto const was_active = is_active();
		auto const visitor = klib::SubVisitor{
			[this](event::Key const& key) { on_key(key); },
			[this](event::Codepoint const codepoint) { on_codepoint(codepoint); },
			[this](event::CursorPos const& cursor) { on_cursor_move(cursor); },
			[this](event::Scroll const& scroll) { on_scroll(scroll); },
		};
		for (auto const& event : events) { std::visit(visitor, event); }
		if (activated != nullptr) { *activated = !was_active && is_active(); }
	}

	void tick(glm::vec2 const framebuffer_size, kvf::Seconds const dt) {
		if (kvf::is_positive(framebuffer_size)) {
			if (m_framebuffer_size != framebuffer_size) {
				m_framebuffer_size = framebuffer_size;
				resize();
			}
		}

		if (m_active) {
			if (m_render_view.position.y < m_show_y) { m_render_view.position.y += m_info.motion.slide_speed * dt.count(); }
			m_render_view.position.y = std::min(m_render_view.position.y, m_show_y);
		} else {
			if (m_render_view.position.y > m_hide_y) { m_render_view.position.y -= m_info.motion.slide_speed * dt.count(); }
			m_render_view.position.y = std::max(m_render_view.position.y, m_hide_y);
		}
		m_input.tick(dt);
	}

	void draw(Renderer& renderer) const {
		if (!m_active && m_render_view.position.y <= m_hide_y) { return; }
		renderer.set_render_area(kvf::uv_rect_v);
		auto const old_view = std::exchange(renderer.view, m_render_view);
		m_background.draw(renderer);
		draw_buffer(renderer);
		m_separator.draw(renderer);
		m_caret.draw(renderer);
		m_input.draw(renderer);
		renderer.view = old_view;
	}

  private:
	struct Builtin : Command {
		explicit Builtin(Impl& impl) : impl(impl) {
			args.push_back(klib::args::named_option(opacity, "o,opacity", {}, &opacity_was_set));
			args.push_back(klib::args::named_option(color, "c,color", {}, &color_was_set));
		}

		[[nodiscard]] auto get_name() const -> std::string_view final { return "console"; }
		[[nodiscard]] auto get_description() const -> std::string_view final { return "console styling"; }
		[[nodiscard]] auto get_args() const -> std::span<klib::args::Arg const> final { return args; }

		void execute(Printer& printer) final {
			if (opacity_was_set) {
				impl.m_background.tint.w = kvf::Color::to_u8(opacity);
			} else if (color_was_set) {
				impl.m_background.tint = kvf::util::color_from_hex(color);
			} else {
				auto text = std::format("opacity: {:.2f}", kvf::Color::to_f32(impl.m_background.tint.w));
				std::format_to(std::back_inserter(text), "\ncolor: {}", kvf::util::to_hex_string(impl.m_background.tint));
				printer.println(text);
			}

			opacity_was_set = false;
			color_was_set = false;
		}

		Impl& impl;

		std::vector<klib::args::Arg> args{};
		float opacity{};
		bool opacity_was_set{};
		std::string_view color{};
		bool color_was_set{};
	};

	struct Help : Command {
		explicit Help(Impl& impl) : impl(impl) {}

		[[nodiscard]] auto get_name() const -> std::string_view final { return "help"; }
		[[nodiscard]] auto get_description() const -> std::string_view final { return "print help text"; }

		void execute(Printer& printer) final {
			auto const help_text = klib::args::HelpString{}(impl.m_args);
			printer.println(help_text);
		}

		Impl& impl;
	};

	void setup(Font& font) {
		m_input.set_interactive(false);

		m_separator.tint = m_info.colors.separator;
		m_background.tint = kvf::Color{0x111111cc};

		m_info.style.text_height = m_input.get_atlas().get_height();
		m_info.motion.slide_speed = std::abs(m_info.motion.slide_speed);
		m_info.motion.scroll_speed = std::abs(m_info.motion.scroll_speed);

		m_caret.create(font.get_atlas(m_info.style.text_height), m_info.style.caret);

		resize();
		m_render_view.position.y = m_hide_y;
	}

	void resize() {
		auto const width = m_framebuffer_size.x;
		m_background.create({width, 0.5f * m_framebuffer_size.y});
		m_separator.create({width, m_info.style.separator_height});
		m_background.transform.position.y = 0.5f * m_background.get_size().y;
		m_separator.transform.position.y = 1.5f * float(m_info.style.text_height);
		m_caret.instance.transform.position = {(-0.5f * m_framebuffer_size.x) + m_info.style.x_pad, 0.5f * float(m_info.style.text_height)};
		m_input.transform.position = m_caret.instance.transform.position;
		m_input.transform.position.x += m_caret.text_x;
		m_hide_y = -0.5f * m_framebuffer_size.y;
		m_show_y = 0.0f;
		m_buffer_max_y = m_separator.transform.position.y + 0.5f * float(m_info.style.text_height);
		m_buffer.position.x = m_caret.instance.transform.position.x;
		set_buffer_y(m_buffer.position.y);
	}

	void draw_buffer(Renderer& renderer) const {
		auto const scissor_y = ((0.5f * m_framebuffer_size.y) - m_separator.transform.position.y) / m_framebuffer_size.y;
		auto const rect = kvf::Rect<>{.rb = {1.0f, scissor_y}};
		renderer.set_scissor_rect(rect);
		m_buffer.draw(renderer);
		renderer.set_scissor_rect(kvf::uv_rect_v);
	}

	void print_input(std::string line) { m_buffer.push({&line, 1}, m_info.colors.input); }

	void move_buffer_y(float const dy) { set_buffer_y(m_buffer.position.y + dy); }

	void set_buffer_y(float const value) {
		auto const max_dy = m_buffer.get_height() - (0.5f * m_framebuffer_size.y);
		if (max_dy < 0.0f) {
			m_buffer.position.y = m_buffer_max_y;
			return;
		}
		m_buffer.position.y = std::clamp(value, -max_dy, m_buffer_max_y);
	}

	void on_key(event::Key const& key) {
		if (key.key == GLFW_KEY_GRAVE_ACCENT && key.action == GLFW_PRESS && key.mods == 0) { toggle_active(); }
		if (!is_active()) { return; }

		if (key.mods == 0) {
			if (key.action == GLFW_PRESS) {
				switch (key.key) {
				case GLFW_KEY_ENTER: on_enter(); break;
				}
			}
			if (key.action != GLFW_RELEASE) {
				switch (key.key) {
				case GLFW_KEY_UP: cycle_up(); break;
				case GLFW_KEY_DOWN: cycle_down(); break;
				case GLFW_KEY_TAB: autocomplete(); break;
				case GLFW_KEY_PAGE_UP: page_up(); break;
				case GLFW_KEY_PAGE_DOWN: page_down(); break;
				}
			}
		}
		m_input.on_key(key);
	}

	void on_codepoint(event::Codepoint const codepoint) {
		if (!is_active() || codepoint == event::Codepoint('`')) { return; }
		m_input.on_codepoint(codepoint);
		stop_cycling();
	}

	void on_cursor_move(event::CursorPos const& cursor_pos) { m_n_cursor_pos = cursor_pos.normalized; }

	void on_scroll(event::Scroll const scroll) {
		if (!is_active() || (m_n_cursor_pos * m_framebuffer_size).y < 0.0f) { return; }
		move_buffer_y(m_info.motion.scroll_speed * -scroll.y);
	}

	void on_enter() {
		auto const text = m_input.get_string();
		if (text.empty()) { return; }

		stop_cycling();
		m_history.emplace_front(text);
		while (m_history.size() > m_info.storage.history) { m_history.pop_back(); }

		auto line = std::format("{} {}", m_info.style.caret, text);
		print_input(line);
		try_run(text);

		m_input.clear();
	}

	void try_run(std::string_view const text) {
		auto const parse_result = klib::args::parse_string(m_args, text, this);
		if (parse_result.early_return()) { return; }

		auto const cmd = parse_result.get_command_name();
		if (auto const it = m_cmd_map.find(cmd); it != m_cmd_map.end()) {
			it->second->execute(*this);
			return;
		}

		printerr(std::format("unrecognized command: '{}'", cmd));
	}

	void start_cycling() {
		if (m_history_index) { return; }
		m_history.emplace_front(m_input.get_string()); // cache current line
	}

	void stop_cycling() {
		if (!m_history_index) { return; }
		m_history_index.reset();
		KLIB_ASSERT(!m_history.empty());
		m_history.pop_front(); // remove cached line
	}

	void cycle_up() {
		if (m_history.empty()) { return; }
		if (!m_history_index) {
			start_cycling();
			m_history_index = 0;
		} else if (*m_history_index + 1 == m_history.size()) {
			return;
		}
		++*m_history_index;
		KLIB_ASSERT(*m_history_index < m_history.size());
		m_input.set_string(m_history.at(*m_history_index));
	}

	void cycle_down() {
		if (!m_history_index || m_history.empty() || *m_history_index == 0) { return; }
		--*m_history_index;
		m_input.set_string(m_history.at(*m_history_index));
	}

	void autocomplete() {
		auto prefix = m_input.get_string();
		if (prefix.empty() || prefix.find_first_of(' ') != std::string_view::npos) { return; }
		auto const candidates = [&] {
			auto ret = std::vector<std::string_view>{};
			ret.reserve(m_cmd_map.size());
			for (auto const& command : m_commands) {
				auto const name = command->get_name();
				if (!name.starts_with(prefix)) { continue; }
				ret.push_back(name);
			}
			return ret;
		}();
		if (candidates.empty()) { return; }
		if (candidates.size() == 1) {
			auto const name = candidates.front();
			m_input.set_string(std::format("{} ", name));
			return;
		}
		prefix = longest_match(candidates, prefix);
		m_input.set_string(std::string{prefix});
		for (auto const candidate : candidates) {
			auto const line = std::format("  {}", candidate);
			println(line);
		}
	}

	void page_up() { move_buffer_y(-((0.5f * m_framebuffer_size.y) - (2.0f * float(m_info.style.text_height)))); }

	void page_down() { move_buffer_y(+((0.5f * m_framebuffer_size.y) - (2.0f * float(m_info.style.text_height)))); }

	CreateInfo m_info;
	glm::vec2 m_framebuffer_size;
	ndc::vec2 m_n_cursor_pos{};

	std::vector<std::unique_ptr<Command>> m_commands{};
	std::vector<klib::args::Arg> m_args{};
	std::unordered_map<std::string_view, Command*> m_cmd_map{};

	std::deque<std::string> m_history{};
	drawable::Quad m_background{};
	drawable::Quad m_separator{};
	Caret m_caret{};
	drawable::InputText m_input;
	Buffer m_buffer;

	float m_hide_y{};
	float m_show_y{};
	float m_buffer_max_y{};
	Transform m_render_view{};
	bool m_active{};

	std::optional<std::size_t> m_history_index{};
};

void Terminal::Deleter::operator()(Impl* ptr) const noexcept { std::default_delete<Impl>{}(ptr); }

Terminal::Terminal(gsl::not_null<Font*> font, glm::vec2 const framebuffer_size, CreateInfo const& create_info)
	: m_impl(new Impl{*font, framebuffer_size, create_info}) {}

void Terminal::add_command(std::unique_ptr<Command> command) { m_impl->add_command(std::move(command)); }

void Terminal::add_command(std::string_view const name, std::string_view const description, std::move_only_function<void(Printer&)> command) {
	m_impl->add_command(name, description, std::move(command));
}

auto Terminal::is_active() const -> bool { return m_impl->is_active(); }

void Terminal::println(std::string_view const text) { m_impl->println(text); }

void Terminal::printerr(std::string_view const text) { m_impl->printerr(text); }

auto Terminal::get_background() const -> kvf::Color { return m_impl->get_background(); }

void Terminal::set_background(kvf::Color const color) { m_impl->set_background(color); }

void Terminal::handle_events(std::span<Event const> events, bool* activated) { m_impl->handle_events(events, activated); }

void Terminal::tick(glm::vec2 framebuffer_size, kvf::Seconds const dt) { m_impl->tick(framebuffer_size, dt); }

void Terminal::draw(Renderer& renderer) const { m_impl->draw(renderer); }
} // namespace le::console
