#include "le2d/console/terminal.hpp"
#include "le2d/console/terminal_builder.hpp"
#include "le2d/drawable/input_text.hpp"
#include "le2d/drawable/shape.hpp"
#include "le2d/text/text_buffer.hpp"
#include "le2d/tweak/registry.hpp"
#include <GLFW/glfw3.h>
#include <klib/assert.hpp>
#include <klib/concepts.hpp>
#include <klib/log.hpp>
#include <klib/str_to_num.hpp>
#include <kvf/util.hpp>
#include <algorithm>
#include <ranges>

namespace le {
namespace console {
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

	void draw(IRenderer& renderer) const {
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

	void draw(IRenderer& renderer) const {
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

auto const log = klib::TaggedLogger{"le::console"};

class Terminal : public ITerminal {
  public:
	explicit Terminal(gsl::not_null<IFont*> font, TerminalCreateInfo const& info, bool const add_builtin_tweaks)
		: m_info(info), m_input(font, to_input_text_params(info)),
		  m_buffer(font->get_atlas(m_info.style.text_height), m_info.storage.buffer, m_info.style.line_spacing) {
		setup(*font);

		if (add_builtin_tweaks) { add_builtins(); }
	}

  private:
	struct Params {
		std::string_view id{};
		std::string_view value{};

		[[nodiscard]] static constexpr auto create(std::string_view const text) -> Params {
			auto i = text.find_first_of(" =");
			if (i == std::string_view::npos) { return Params{.id = text}; }
			auto value = text.substr(i + 1);
			while (!value.empty() && value.front() == ' ') { value = value.substr(1); }
			return Params{.id = text.substr(0, i), .value = value};
		}
	};

	static constexpr auto to_input_text_params(TerminalCreateInfo const& in) -> InputTextParams {
		return InputTextParams{
			.height = in.style.text_height,
			.cursor_symbol = in.style.cursor,
			.cursor_color = in.colors.cursor,
			.blink_period = in.motion.blink_period,
		};
	}

	[[nodiscard]] auto is_null() const -> bool final { return false; }

	[[nodiscard]] auto is_active() const -> bool final { return m_active; }

	void toggle_active() final {
		m_active = !m_active;
		m_input.set_interactive(m_active);
	}

	void add_tweakable(std::string_view const id, gsl::not_null<ITweakable*> tweakable) final {
		m_tweak_registry.add_tweakable(id, tweakable);
		refresh_tweak_entries();
	}
	void remove_tweakable(std::string_view const id) final {
		m_tweak_registry.remove_tweakable(id);
		refresh_tweak_entries();
	}

	[[nodiscard]] auto get_background() const -> kvf::Color final { return m_background.tint; }

	void set_background(kvf::Color const color) final { m_background.tint = color; }

	void println(std::string_view const text) final {
		Buffer::Printer{m_buffer}.print(text, m_info.colors.output);
		log.info("{}", text);
	}

	void printerr(std::string_view const text) final {
		Buffer::Printer{m_buffer}.print(text, m_info.colors.error);
		log.error("{}", text);
	}

	auto handle_events(glm::vec2 const framebuffer_size, std::span<Event const> events) -> StateChange final {
		auto const was_active = is_active();
		auto const visitor = klib::SubVisitor{
			[this](event::Key const& key) { on_key(key); },
			[this](event::Codepoint const codepoint) { on_codepoint(codepoint); },
			[this](event::CursorPos const& cursor) { on_cursor_move(cursor); },
			[this](event::Scroll const& scroll) { on_scroll(scroll); },
		};
		for (auto const& event : events) { std::visit(visitor, event); }

		if (kvf::is_positive(framebuffer_size) && m_framebuffer_size != framebuffer_size) {
			m_framebuffer_size = framebuffer_size;
			resize();
		}

		if (!was_active && is_active()) { return StateChange::Activated; }
		if (was_active && !is_active()) { return StateChange::Deactivated; }
		return StateChange::None;
	}

	void tick(kvf::Seconds const dt) final {
		if (m_active) {
			if (m_render_view.position.y < m_show_y) { m_render_view.position.y += m_info.motion.slide_speed * dt.count(); }
			m_render_view.position.y = std::min(m_render_view.position.y, m_show_y);
		} else {
			if (m_render_view.position.y > m_hide_y) { m_render_view.position.y -= m_info.motion.slide_speed * dt.count(); }
			m_render_view.position.y = std::max(m_render_view.position.y, m_hide_y);
		}
		m_input.tick(dt);
	}

	void draw(IRenderer& renderer) const final {
		if (!m_active && m_render_view.position.y <= m_hide_y) { return; }
		auto const old_view = std::exchange(renderer.view, m_render_view);
		auto const old_viewport = std::exchange(renderer.viewport, viewport::Dynamic{});
		m_background.draw(renderer);
		draw_buffer(renderer);
		m_separator.draw(renderer);
		m_caret.draw(renderer);
		m_input.draw(renderer);
		renderer.view = old_view;
		renderer.viewport = old_viewport;
	}

	void setup(IFont& font) {
		m_input.set_interactive(false);

		m_separator.tint = m_info.colors.separator;
		m_background.tint = kvf::Color{0x301020cc}.to_linear();

		m_info.style.text_height = m_input.get_atlas().get_height();
		m_info.motion.slide_speed = std::abs(m_info.motion.slide_speed);
		m_info.motion.scroll_speed = std::abs(m_info.motion.scroll_speed);

		m_caret.create(font.get_atlas(m_info.style.text_height), m_info.style.caret);

		resize();
		m_render_view.position.y = m_hide_y;
	}

	void add_builtins() {
		m_opacity.on_set([this](float const value) {
			if (value < 0.0f || value > 1.0f) { return false; }
			m_background.tint.w = kvf::Color::to_u8(value);
			return true;
		});
		add_tweakable("console.opacity", &m_opacity);
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

	void draw_buffer(IRenderer& renderer) const {
		auto const scissor_y = ((0.5f * m_framebuffer_size.y) - m_separator.transform.position.y) / m_framebuffer_size.y;
		auto const rect = kvf::Rect<>{.rb = {1.0f, scissor_y}};
		renderer.scissor_rect = rect;
		m_buffer.draw(renderer);
		renderer.scissor_rect = kvf::uv_rect_v;
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
		auto const params = Params::create(text);
		if (params.id.empty()) { return; }
		auto tweakable = m_tweak_registry.find_tweakable(params.id);
		if (!tweakable) {
			printerr(std::format("unrecognized identifier: {}", params.id));
			return;
		}
		if (!params.value.empty() && !tweakable->assign(params.value)) {
			printerr(std::format("invalid {} [{}] value: {}", params.id, tweakable->type_name(), params.value));
			return;
		}
		println(std::format("{} [{}]: {}", params.id, tweakable->type_name(), tweakable->as_string()));
	}

	void refresh_tweak_entries() {
		m_tweak_entries.clear();
		m_tweak_registry.fill_entries(m_tweak_entries);
	}

	void append_tweakables(std::string& out) const {
		if (m_tweak_entries.empty()) {
			out = "[no Tweakables added]";
			return;
		}

		for (auto const& entry : m_tweak_entries) { std::format_to(std::back_inserter(out), "  {} [{}]\n", entry.id, entry.tweakable->type_name()); }
		out.pop_back();
	}

	void print_tweakables() {
		auto text = std::string{};
		append_tweakables(text);
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
		if (prefix.empty()) {
			print_tweakables();
			return;
		}

		fill_autocomplete_candidates(prefix);
		if (m_candidates_buffer.empty()) { return; }
		if (m_candidates_buffer.size() == 1) {
			auto const name = m_candidates_buffer.front();
			m_input.set_string(std::format("{} ", name));
			return;
		}

		prefix = longest_match(m_candidates_buffer, prefix);
		m_input.set_string(std::string{prefix});

		auto text = std::string{"\n"};
		for (auto const candidate : m_candidates_buffer) { std::format_to(std::back_inserter(text), "  {}\n", candidate); }
		text.pop_back();
		println(text);
	}

	void fill_autocomplete_candidates(std::string_view const prefix) {
		m_candidates_buffer.clear();
		m_candidates_buffer.reserve(m_tweak_entries.size());
		for (auto const& entry : m_tweak_entries) {
			if (!prefix.empty() && !entry.id.starts_with(prefix)) { continue; }
			m_candidates_buffer.push_back(entry.id);
		}
	}

	void page_up() { move_buffer_y(-((0.5f * m_framebuffer_size.y) - (2.0f * float(m_info.style.text_height)))); }

	void page_down() { move_buffer_y(+((0.5f * m_framebuffer_size.y) - (2.0f * float(m_info.style.text_height)))); }

	TerminalCreateInfo m_info;
	glm::vec2 m_framebuffer_size{400.0f, 300.0f};
	ndc::vec2 m_n_cursor_pos{};

	tweak::Registry m_tweak_registry{};
	std::vector<tweak::Registry::Entry> m_tweak_entries{};
	std::vector<std::string_view> m_candidates_buffer{};
	Tweakable<float> m_opacity{};

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

struct NullTerminal : ITerminal {
	[[nodiscard]] auto is_null() const -> bool final { return true; }

	[[nodiscard]] auto is_active() const -> bool final { return false; }
	void toggle_active() final {}

	void add_tweakable(std::string_view /*id*/, gsl::not_null<ITweakable*> /*tweakable*/) final {}
	void remove_tweakable(std::string_view /*id*/) final {}

	void println(std::string_view /*text*/) final {}
	void printerr(std::string_view /*text*/) final {}

	[[nodiscard]] auto get_background() const -> kvf::Color final { return {}; }
	void set_background(kvf::Color /*color*/) final {}

	auto handle_events(glm::vec2 /*framebuffer_size*/, std::span<Event const> /*events*/) -> StateChange final { return StateChange::None; }

	void tick(kvf::Seconds /*dt*/) final {}
	void draw(IRenderer& /*renderer*/) const final {}
};
} // namespace

auto TerminalBuilder::build(gsl::not_null<IFont*> font) const -> std::unique_ptr<ITerminal> {
	return std::make_unique<Terminal>(font, create_info, add_builtin_tweaks);
}
} // namespace console

auto console::build_null_terminal() -> std::unique_ptr<ITerminal> { return std::make_unique<NullTerminal>(); }
} // namespace le
