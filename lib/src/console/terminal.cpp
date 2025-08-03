#include <GLFW/glfw3.h>
#include <klib/args/parse.hpp>
#include <klib/assert.hpp>
#include <klib/concepts.hpp>
#include <klib/log.hpp>
#include <klib/str_to_num.hpp>
#include <kvf/util.hpp>
#include <le2d/console/terminal.hpp>
#include <le2d/console/tweak.hpp>
#include <le2d/drawable/input_text.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/text/text_buffer.hpp>
#include <algorithm>
#include <functional>
#include <map>
#include <ranges>

namespace le::console {
namespace {
struct Caret {
	TextGeometry geometry{};
	ITexture const* texture{};
	RenderInstance instance{};
	float text_x{};

	void create(IFontAtlas& atlas, char const c) {
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

	explicit Buffer(IFontAtlas& atlas, std::size_t const limit, float n_line_spacing) : m_text_buffer(&atlas, limit, n_line_spacing) {}

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
} // namespace

struct Terminal::Impl : IPrinter {
	static constexpr auto to_input_text_params(CreateInfo const& in) {
		return InputTextParams{
			.height = in.style.text_height,
			.cursor_symbol = in.style.cursor,
			.cursor_color = in.colors.cursor,
			.blink_period = in.motion.blink_period,
		};
	}

	inline static auto const log = klib::TaggedLogger{"le::console"};

	explicit Impl(IFont& font, glm::vec2 const framebuffer_size, CreateInfo const& info)
		: m_info(info), m_framebuffer_size(framebuffer_size), m_input(&font, to_input_text_params(info)),
		  m_buffer(font.get_atlas(m_info.style.text_height), m_info.storage.buffer, m_info.style.line_spacing) {
		setup(font);
		add_command("console.opacity", std::make_unique<Opacity>(*this));
		add_command("help", std::make_unique<Help>(*this));
	}

	void toggle_active() {
		m_active = !m_active;
		m_input.set_interactive(m_active);
	}

	[[nodiscard]] auto is_active() const -> bool { return m_active; }

	void add_command(std::string_view const name, std::unique_ptr<ICommand> command) {
		if (!command || name.empty()) { return; }
		m_command_map.insert_or_assign(name, std::move(command));
		fill_command_args();
	}

	void remove_command(std::string_view const name) {
		if (name.empty()) { return; }
		m_command_map.erase(name);
		fill_command_args();
	}

	[[nodiscard]] auto get_background() const -> kvf::Color { return m_background.tint; }

	void set_background(kvf::Color const color) { m_background.tint = color; }

	void println(std::string_view const text) final {
		Buffer::Printer{m_buffer}.print(text, m_info.colors.output);
		log.info("{}", text);
	}

	void printerr(std::string_view const text) final {
		Buffer::Printer{m_buffer}.print(text, m_info.colors.error);
		log.error("{}", text);
	}

	void handle_events(glm::vec2 const framebuffer_size, std::span<Event const> events, bool* activated) {
		auto const was_active = is_active();
		auto const visitor = klib::SubVisitor{
			[this](event::Key const& key) { on_key(key); },
			[this](event::Codepoint const codepoint) { on_codepoint(codepoint); },
			[this](event::CursorPos const& cursor) { on_cursor_move(cursor); },
			[this](event::Scroll const& scroll) { on_scroll(scroll); },
		};
		for (auto const& event : events) { std::visit(visitor, event); }
		if (activated != nullptr) { *activated = !was_active && is_active(); }

		if (kvf::is_positive(framebuffer_size) && m_framebuffer_size != framebuffer_size) {
			m_framebuffer_size = framebuffer_size;
			resize();
		}
	}

	void tick(kvf::Seconds const dt) {
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
	struct Opacity : Tweak<float> {
		explicit Opacity(Impl& impl) : m_impl(impl) {}

	  private:
		auto set(float value) -> bool final {
			value = std::clamp(value, 0.0f, 1.0f);
			m_impl.m_background.tint.w = kvf::Color::to_u8(value);
			return true;
		}

		[[nodiscard]] auto get() const -> float final { return kvf::Color::to_f32(m_impl.m_background.tint.w); }

		Impl& m_impl;
	};

	struct Help : ICommand {
		explicit Help(Impl& impl) : m_impl(impl) {}

	  private:
		[[nodiscard]] auto get_help_text() const -> std::string_view final { return "display this help text"; }

		void run(IPrinter& /*printer*/) final { m_impl.print_help(); }

		Impl& m_impl;
	};

	struct Params {
		std::string_view name{};
		std::string_view value{};

		[[nodiscard]] static constexpr auto create(std::string_view const text) -> Params {
			auto i = text.find_first_of(" =");
			if (i == std::string_view::npos) { return Params{.name = text}; }
			auto value = text.substr(i + 1);
			while (!value.empty() && value.front() == ' ') { value = value.substr(1); }
			return Params{.name = text.substr(0, i), .value = value};
		}
	};

	void setup(IFont& font) {
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

	void print_input(std::string line) {
		log.info("{}", line);
		m_buffer.push({&line, 1}, m_info.colors.input);
	}

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
		auto const parse_info = klib::args::ParseStringInfo{
			.printer = this,
			.flags = klib::args::ParseFlag::OmitDefaultValues,
		};
		auto const parse_result = klib::args::parse_string(parse_info, m_command_args, text);
		if (parse_result.early_return()) { return; }

		auto const command_name = parse_result.get_command_name();
		auto const it = m_command_map.find(command_name);
		if (it == m_command_map.end()) {
			printerr(std::format("unrecognized Command: '{}'", command_name));
			return;
		}

		auto& command = *it->second;
		command.run(*this);
	}

	auto try_builtin(std::string_view const name) -> bool {
		auto const it = m_builtin_map.find(name);
		if (it == m_builtin_map.end()) { return false; }
		it->second();
		return true;
	}

	void append_command_names(std::string& out) const {
		if (m_command_map.empty()) {
			out = "[no Commands added]";
			return;
		}

		for (auto const& [name, _] : m_command_map) { std::format_to(std::back_inserter(out), "  {}\n", name); }
		out.pop_back();
	}

	void print_command_names() {
		auto text = std::string{};
		append_command_names(text);
		println(text);
	}

	void print_help() {
		auto const text = klib::args::HelpString{}(m_command_args);
		println(text);
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
		fill_autocomplete_candidates(prefix);

		if (m_candidates_buffer.empty()) { return; }
		if (m_candidates_buffer.size() == 1) {
			auto const name = m_candidates_buffer.front();
			m_input.set_string(std::format("{} ", name));
			return;
		}

		if (!prefix.empty()) {
			prefix = longest_match(m_candidates_buffer, prefix);
			m_input.set_string(std::string{prefix});
		}
		auto text = std::string{"\n"};
		for (auto const candidate : m_candidates_buffer) { std::format_to(std::back_inserter(text), "  {}\n", candidate); }
		text.pop_back();
		println(text);
	}

	void fill_autocomplete_candidates(std::string_view const prefix) {
		m_candidates_buffer.clear();
		m_candidates_buffer.reserve(m_builtin_map.size() + m_command_map.size());
		auto const push_names = [this, prefix](auto const& map) {
			for (auto const& [name, _] : map) {
				if (!prefix.empty() && !name.starts_with(prefix)) { continue; }
				m_candidates_buffer.push_back(name);
			}
		};
		push_names(m_builtin_map);
		push_names(m_command_map);
	}

	void fill_command_args() {
		m_command_args.clear();
		m_command_args.reserve(m_command_map.size());
		for (auto const& [name, command] : m_command_map) {
			m_command_args.push_back(klib::args::command(command->get_args(), name, command->get_help_text()));
		}
	}

	void page_up() { move_buffer_y(-((0.5f * m_framebuffer_size.y) - (2.0f * float(m_info.style.text_height)))); }

	void page_down() { move_buffer_y(+((0.5f * m_framebuffer_size.y) - (2.0f * float(m_info.style.text_height)))); }

	CreateInfo m_info;
	glm::vec2 m_framebuffer_size;
	ndc::vec2 m_n_cursor_pos{};

	std::map<std::string_view, std::move_only_function<void()>> m_builtin_map{};
	std::map<std::string_view, std::unique_ptr<ICommand>> m_command_map{};
	std::vector<klib::args::Arg> m_command_args{};
	std::vector<std::string_view> m_candidates_buffer{};

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

Terminal::Terminal(gsl::not_null<IFont*> font, glm::vec2 const framebuffer_size, CreateInfo const& create_info)
	: m_impl(new Impl{*font, framebuffer_size, create_info}) {}

auto Terminal::is_active() const -> bool { return m_impl->is_active(); }

void Terminal::add_command(std::string_view const name, std::unique_ptr<ICommand> command) { m_impl->add_command(name, std::move(command)); }

void Terminal::remove_command(std::string_view const name) { m_impl->remove_command(name); }

void Terminal::println(std::string_view const text) { m_impl->println(text); }

void Terminal::printerr(std::string_view const text) { m_impl->printerr(text); }

auto Terminal::get_background() const -> kvf::Color { return m_impl->get_background(); }

void Terminal::set_background(kvf::Color const color) { m_impl->set_background(color); }

void Terminal::handle_events(glm::vec2 const framebuffer_size, std::span<Event const> events, bool* activated) {
	m_impl->handle_events(framebuffer_size, events, activated);
}

void Terminal::tick(kvf::Seconds const dt) { m_impl->tick(dt); }

void Terminal::draw(Renderer& renderer) const { m_impl->draw(renderer); }
} // namespace le::console
