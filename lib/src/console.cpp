#include <GLFW/glfw3.h>
#include <klib/assert.hpp>
#include <klib/concepts.hpp>
#include <klib/str_to_num.hpp>
#include <kvf/util.hpp>
#include <le2d/console.hpp>
#include <le2d/drawables/input_text.hpp>
#include <le2d/drawables/shape.hpp>
#include <algorithm>
#include <deque>

namespace le::console {
namespace {
struct Caret {
	VertexArray verts{};
	Texture const* texture{};
	RenderInstance instance{};
	float text_x{};

	void create(FontAtlas& atlas, char const c) {
		auto const text = std::format("{} ", c);
		auto layouts = std::vector<kvf::ttf::GlyphLayout>{};
		layouts.reserve(2);
		text_x = atlas.push_layouts(layouts, text).x;
		write_glyphs(verts, layouts);
		texture = &atlas.get_texture();
	}

	void draw(Renderer& renderer) const {
		auto const primitive = Primitive{
			.vertices = verts.vertices,
			.indices = verts.indices,
			.texture = texture,
		};
		renderer.draw(primitive, {&instance, 1});
	}
};

struct Line {
	std::string text{};
	kvf::Color color{};
};

struct Buffer {
	explicit Buffer(FontAtlas& atlas, std::size_t const limit, float n_line_height) : m_atlas(atlas), m_limit(limit), m_n_line_height(n_line_height) {}

	void push(std::string text, kvf::Color color) {
		m_lines.push_front(Line{.text = std::move(text), .color = color});
		while (m_lines.size() > m_limit) { m_lines.pop_back(); }
		generate();
	}

	[[nodiscard]] auto get_height() const -> float { return m_height; }

	void draw(Renderer& renderer) const {
		auto const primitive = Primitive{
			.vertices = m_verts.vertices,
			.indices = m_verts.indices,
			.texture = &m_atlas.get_texture(),
		};
		auto const instance = RenderInstance{.transform = {.position = position}};
		renderer.draw(primitive, {&instance, 1});
	}

	glm::vec2 position{};

  private:
	void generate() {
		m_verts.clear();
		m_height = 0.0f;

		auto pos = glm::vec2{};
		for (auto const& line : m_lines) {
			m_layouts.clear();
			m_atlas.push_layouts(m_layouts, line.text);
			write_glyphs(m_verts, m_layouts, pos, line.color);
			pos.x = 0.0f;
			pos.y += m_n_line_height * float(m_atlas.get_height());
			m_height = pos.y;
		}
	}

	FontAtlas& m_atlas;
	std::size_t m_limit;
	float m_n_line_height;

	std::deque<Line> m_lines{};

	std::vector<kvf::ttf::GlyphLayout> m_layouts{};
	VertexArray m_verts{};
	float m_height{};
};

constexpr auto get_next_arg(std::string_view& out_arg, std::string_view& out_remain) -> bool {
	if (out_remain.empty()) { return false; }
	auto const i = out_remain.find_first_of(' ');
	if (i == std::string_view::npos) {
		out_arg = out_remain;
		out_remain = {};
	} else {
		out_arg = out_remain.substr(0, i);
		out_remain = out_remain.substr(i + 1);
	}
	return true;
}

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

struct Terminal::Impl {
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
		add_builtins();
	}

	void toggle_active() {
		m_active = !m_active;
		m_input.set_interactive(m_active);
	}

	[[nodiscard]] auto is_active() const -> bool { return m_active; }

	void add_command(std::string_view const name, Command command) {
		if (name.empty() || !command) { return; }
		auto const index = m_commands.size();
		m_commands.push_back(std::move(command));
		m_cmd_indices.insert_or_assign(name, index);
	}

	[[nodiscard]] auto get_background() const -> kvf::Color { return m_background.instance.tint; }

	void set_background(kvf::Color const color) { m_background.instance.tint = color; }

	void resize(glm::vec2 const framebuffer_size) {
		if (!kvf::is_positive(framebuffer_size)) { return; }
		m_framebuffer_size = framebuffer_size;
		resize();
	}

	void on_key(event::Key const& key) {
		if (key.mods == 0) {
			if (key.action == GLFW_PRESS) {
				switch (key.key) {
				case GLFW_KEY_GRAVE_ACCENT: toggle_active(); break;
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
		if (!m_active || codepoint == event::Codepoint('`')) { return; }
		m_input.on_codepoint(codepoint);
		stop_cycling();
	}

	void on_cursor_move(event::CursorPos const& cursor_pos) { m_n_cursor_pos = cursor_pos.normalized; }

	void on_scroll(event::Scroll const scroll) {
		if ((m_n_cursor_pos * m_framebuffer_size).y < 0.0f) { return; }
		move_buffer_y(m_info.motion.scroll_speed * -scroll.y);
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
	struct StreamImpl : Stream {
		explicit StreamImpl(Impl& terminal, std::string_view const args) : m_terminal(terminal), m_args(args) {}

		auto next_arg(std::string_view& out) -> bool final { return get_next_arg(out, m_args); }

		auto next_arg() -> std::string_view final {
			auto ret = std::string_view{};
			if (!next_arg(ret)) { return {}; }
			return ret;
		}

		void println(std::string_view line) final { m_terminal.println(line); }
		void printerr(std::string_view line) final { m_terminal.printerr(line); }

	  private:
		Impl& m_terminal;
		std::string_view m_args;
	};

	void setup(Font& font) {
		m_input.set_interactive(false);

		m_separator.instance.tint = m_info.colors.separator;
		m_background.instance.tint = kvf::Color{0x111111cc};

		m_info.style.text_height = m_input.get_atlas().get_height();
		m_info.motion.slide_speed = std::abs(m_info.motion.slide_speed);
		m_info.motion.scroll_speed = std::abs(m_info.motion.scroll_speed);

		m_text_params.height = m_info.style.text_height;
		m_text_params.expand = TextExpand::eRight;

		m_caret.create(font.get_atlas(m_info.style.text_height), m_info.style.caret);

		resize();
		m_render_view.position.y = m_hide_y;
	}

	void add_builtins() {
		add_command("help", [this](Stream& /*unused*/) { print_help(); });

		auto cmd_opacity = [this](Stream& stream) {
			auto const value = stream.next_arg();
			if (value.empty()) {
				stream.println(std::format("{}", kvf::Color::to_f32(m_background.instance.tint.w)));
				return;
			}
			auto const fvalue = klib::str_to_num(value, -1.0f);
			if (fvalue < 0.0f) {
				stream.printerr(std::format("invalid opacity value: '{}'", value));
				return;
			}
			m_background.instance.tint.w = kvf::Color::to_u8(fvalue);
		};
		add_command("console.opacity", cmd_opacity);

		auto cmd_background = [this](Stream& stream) {
			auto const value = stream.next_arg();
			if (value.empty()) {
				auto const tmp = kvf::util::to_hex_string(m_background.instance.tint);
				auto const srgb = m_background.instance.tint;
				auto const str = kvf::util::to_hex_string(srgb);
				stream.println(std::format("{}", str));
				return;
			}
			m_background.instance.tint = kvf::util::color_from_hex(value);
		};
		add_command("console.background", cmd_background);
	}

	void resize() {
		auto const width = m_framebuffer_size.x;
		m_background.create({width, 0.5f * m_framebuffer_size.y});
		m_separator.create({width, m_info.style.separator_height});
		m_background.instance.transform.position.y = 0.5f * m_background.get_size().y;
		m_separator.instance.transform.position.y = 1.5f * float(m_info.style.text_height);
		m_caret.instance.transform.position = {(-0.5f * m_framebuffer_size.x) + m_info.style.x_pad, 0.5f * float(m_info.style.text_height)};
		m_input.instance.transform.position = m_caret.instance.transform.position;
		m_input.instance.transform.position.x += m_caret.text_x;
		m_hide_y = -0.5f * m_framebuffer_size.y;
		m_show_y = 0.0f;
		m_buffer_max_y = m_separator.instance.transform.position.y + 0.5f * float(m_info.style.text_height);
		m_buffer.position.x = m_caret.instance.transform.position.x;
		set_buffer_y(m_buffer.position.y);
	}

	void draw_buffer(Renderer& renderer) const {
		auto const scissor_y = (0.5f * m_framebuffer_size.y) - m_separator.instance.transform.position.y;
		auto scissor = vk::Rect2D{vk::Offset2D{}, vk::Extent2D{std::uint32_t(m_framebuffer_size.x), std::uint32_t(scissor_y)}};
		renderer.command_buffer().setScissor(0, scissor);
		m_buffer.draw(renderer);
		scissor.extent.height = std::uint32_t(m_framebuffer_size.y);
		renderer.command_buffer().setScissor(0, scissor);
	}

	void print(std::string_view const line, kvf::Color const color) { m_buffer.push(std::string{line}, color); }

	void println(std::string_view const line) { print(line, m_info.colors.output); }

	void printerr(std::string_view const line) { print(line, m_info.colors.error); }

	void print_help() {
		if (m_commands.empty()) {
			println("[no commands registered]");
			return;
		}
		println("registered commands:");
		for (auto const& [name, _] : m_cmd_indices) {
			auto const line = std::format("  {}", name);
			println(line);
		}
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

	void on_enter() {
		auto const text = m_input.get_string();
		if (text.empty()) { return; }

		stop_cycling();
		m_history.emplace_front(text);
		while (m_history.size() > m_info.storage.history) { m_history.pop_back(); }

		auto const line = std::format("{} {}", m_info.style.caret, text);
		print(line, m_info.colors.input);
		try_run(text);

		m_input.clear();
	}

	void try_run(std::string_view const text) {
		auto stream = StreamImpl{*this, text};
		auto const cmd = stream.next_arg();
		if (cmd.empty()) { return; }

		if (auto const it = m_cmd_indices.find(cmd); it != m_cmd_indices.end()) {
			m_commands.at(it->second)(stream);
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
			ret.reserve(m_cmd_indices.size());
			for (auto const& [name, _] : m_cmd_indices) {
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
	TextParams m_text_params;

	std::vector<Command> m_commands{};
	std::unordered_map<std::string_view, std::size_t> m_cmd_indices{};

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

void Terminal::add_command(std::string_view const name, Command command) { m_impl->add_command(name, std::move(command)); }

auto Terminal::is_active() const -> bool { return m_impl->is_active(); }

auto Terminal::get_background() const -> kvf::Color { return m_impl->get_background(); }

void Terminal::set_background(kvf::Color const color) { m_impl->set_background(color); }

void Terminal::resize(glm::ivec2 const framebuffer_size) { m_impl->resize(framebuffer_size); }

void Terminal::on_key(event::Key const& key) { m_impl->on_key(key); }

void Terminal::on_codepoint(event::Codepoint codepoint) { m_impl->on_codepoint(codepoint); }

void Terminal::on_cursor_move(event::CursorPos const& cursor_pos) { m_impl->on_cursor_move(cursor_pos); }

void Terminal::on_scroll(event::Scroll scroll) { m_impl->on_scroll(scroll); }

void Terminal::tick(kvf::Seconds const dt) { m_impl->tick(dt); }

void Terminal::draw(Renderer& renderer) const { m_impl->draw(renderer); }
} // namespace le::console
